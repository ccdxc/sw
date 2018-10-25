import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { FormArray, Validators } from '@angular/forms';
import { ErrorStateMatcher, MatRadioChange } from '@angular/material';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringAlertDestination, MonitoringAlertDestination, MonitoringAuthConfig, MonitoringPrivacyConfig, MonitoringSNMPTrapServer, MonitoringSyslogExport } from '@sdk/v1/models/generated/monitoring';
import { SelectItem, MessageService } from 'primeng/primeng';
import { Observable } from 'rxjs/Observable';
import { BaseComponent } from '@app/components/base/base.component';

// Creating manually, this will come from venice-sdk once Ranjith's proto changes go in
enum CredentialMethod {
  AUTHTYPE_NONE = 'None',
  'AUTHTYPE_USERNAMEPASSWORD' = 'Username/Password',
  AUTHTYPE_TOKEN = 'Token',
  AUTHTYPE_CERTS = 'Certs'
}

@Component({
  selector: 'app-newdestination',
  templateUrl: './newdestination.component.html',
  styleUrls: ['./newdestination.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewdestinationComponent extends BaseComponent implements OnInit, AfterViewInit {
  enableSnmpTrap: boolean = false;

  newDestination: MonitoringAlertDestination;

  @Input() isInline: boolean = false;
  @Input() destinationData: IMonitoringAlertDestination;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  versionOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringSNMPTrapServer.propInfo['version'].enum);

  authAlgoOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringAuthConfig.propInfo['algo'].enum);

  encryptAlgoOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringPrivacyConfig.propInfo['algo'].enum);

  syslogFormatOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringSyslogExport.propInfo['format'].enum);

  syslogCredentialOptions: SelectItem[] = Utility.convertEnumToSelectItem(CredentialMethod);

  // This field should eventually come from the from group once the necessary proto changes go in
  selectedCredentialMethod = 'AUTHTYPE_USERNAMEPASSWORD';

  oldButtons: ToolbarButton[] = [];

  errorChecker = new ErrorStateMatcher();

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService);
  }


  ngOnInit() {
    if (this.destinationData != null) {
      this.newDestination = new MonitoringAlertDestination(this.destinationData);
    } else {
      this.newDestination = new MonitoringAlertDestination();
    }

    if (this.isInline) {
      // disable name field
      this.newDestination.$formGroup.get(['meta', 'name']).disable();
    } else {
      // Name field can't be blank
      this.newDestination.$formGroup.get(['meta', 'name']).setValidators(Validators.required);
    }
    // Check if there is a syslog server config,
    // if not, we need to add one so that it shows in the form
    const syslogs = this.newDestination.$formGroup.get(['spec', 'syslog-servers']) as FormArray;
    if (syslogs.length === 0) {
      syslogs.insert(0, new MonitoringSyslogExport().$formGroup);
    }

    // Check if there is a snmp trap server config,
    // if not, we need to add one so that it shows in the form
    if (this.enableSnmpTrap) {
      const snmpArray = this.newDestination.$formGroup.get(['spec', 'snmp-trap-servers']) as FormArray;
      if (snmpArray.length === 0) {
        snmpArray.insert(0, new MonitoringSNMPTrapServer().$formGroup);
      }
    }
  }

  ngAfterViewInit() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary eventalertpolicies-button',
          text: 'CREATE DESTINATION',
          callback: () => { this.saveDestination(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-neutral eventalertpolicies-button',
          text: 'CANCEL',
          callback: () => { this.cancelDestination(); }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }

  computeButtonClass() {
    if (this.newDestination.$formGroup.get('meta.name').status === 'VALID') {
      return '';
    } else {
      return 'global-button-disabled';
    }
  }

  getSelectedCredentialMethod(syslog: any): string {
    return syslog.value.target.credentials['auth-type'];
  }

  /**
   * Sets the previously saved toolbar buttons
   * They should have been saved in the ngOnInit when we are inline.
   */
  setPreviousToolbar() {
    if (this.oldButtons != null) {
      const currToolbar = this._controllerService.getToolbarData();
      currToolbar.buttons = this.oldButtons;
      this._controllerService.setToolbarData(currToolbar);
    }
  }


  saveDestination() {
    let handler: Observable<{ body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number }>;
    const destination = this.newDestination.getFormGroupValues();
    if (this.isInline) {
      handler = this._monitoringService.UpdateAlertDestination(this.newDestination.meta.name, destination);
    } else {
      handler = this._monitoringService.AddAlertDestination(destination);
    }
    handler.subscribe(
      (response) => {
        if (this.isInline) {
          this.invokeSuccessToaster('Update Successful', 'Updated destination ' + this.newDestination.meta.name);
        } else {
          this.invokeSuccessToaster('Creation Successful', 'Created destination ' + destination.meta.name);
        }
        this.cancelDestination();
      },
      (error) => {
        if (this.isInline) {
          this.invokeRESTErrorToaster('Update Failed', error);
        } else {
          this.invokeRESTErrorToaster('Creation Failed', error);
        }
      }
    );
  }

  cancelDestination() {
    if (!this.isInline) {
      // Need to reset the toolbar that we changed
      this.setPreviousToolbar();
    }
    this.formClose.emit();
  }


  addSnmpTrapConfig() {
    const snmpArray = this.newDestination.$formGroup.get(['spec', 'snmp-trap-servers']) as FormArray;
    snmpArray.insert(0, new MonitoringSNMPTrapServer().$formGroup);
  }

  removeSnmpTrapConfig(index) {
    const snmpFormArray = this.newDestination.$formGroup.get(['spec', 'snmp-trap-servers']) as FormArray;
    if (snmpFormArray.length > 1) {
      snmpFormArray.removeAt(index);
    }
  }

  addSyslogConfig() {
    const snmpArray = this.newDestination.$formGroup.get(['spec', 'syslog-servers']) as FormArray;
    snmpArray.insert(0, new MonitoringSyslogExport().$formGroup);
  }

  removeSyslogConfig(index) {
    const snmpFormArray = this.newDestination.$formGroup.get(['spec', 'syslog-servers']) as FormArray;
    if (snmpFormArray.length > 1) {
      snmpFormArray.removeAt(index);
    }
  }

}
