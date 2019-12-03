import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild } from '@angular/core';
import { FormArray } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringAlertDestination, MonitoringAlertDestination, MonitoringAuthConfig, MonitoringPrivacyConfig, MonitoringSNMPTrapServer, MonitoringSyslogExport } from '@sdk/v1/models/generated/monitoring';
import { SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { BaseComponent } from '@app/components/base/base.component';
import { SyslogComponent } from '@app/components/shared/syslog/syslog.component';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { required } from '@sdk/v1/utils/validators';

@Component({
  selector: 'app-newdestination',
  templateUrl: './newdestination.component.html',
  styleUrls: ['./newdestination.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewdestinationComponent extends BaseComponent implements OnInit, AfterViewInit {
  @ViewChild('syslogComponent') syslogServers: SyslogComponent;
  @ViewChild('fieldSelector') fieldSelector: FieldselectorComponent;
  enableSnmpTrap: boolean = false;

  newDestination: MonitoringAlertDestination;

  @Input() isInline: boolean = false;
  @Input() destinationData: IMonitoringAlertDestination;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  versionOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringSNMPTrapServer.propInfo['version'].enum);

  authAlgoOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringAuthConfig.propInfo['algo'].enum);

  encryptAlgoOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringPrivacyConfig.propInfo['algo'].enum);

  syslogFormatOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringSyslogExport.propInfo['format'].enum);

  // This field should eventually come from the from group once the necessary proto changes go in
  selectedCredentialMethod = 'AUTHTYPE_USERNAMEPASSWORD';

  oldButtons: ToolbarButton[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService);
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
          cssClass: 'global-button-primary eventalertpolicies-button eventalertpolicies-button-destination-SAVE',
          text: 'CREATE DESTINATION',
          callback: () => { this.saveDestination(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-neutral eventalertpolicies-button eventalertpolicies-button-destination-CANCEL',
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
    const destination: IMonitoringAlertDestination = this.newDestination.getFormGroupValues();
    destination.spec['syslog-export'] = this.syslogServers.getValues();
    // Commenting out since backend doesn't support it currently
    if (this.isInline) {
      handler = this._monitoringService.UpdateAlertDestination(this.newDestination.meta.name, destination);
    } else {
      handler = this._monitoringService.AddAlertDestination(destination);
    }
    handler.subscribe(
      (response) => {
        if (this.isInline) {
          this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Updated destination ' + this.newDestination.meta.name);
        } else {
          this._controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Created destination ' + destination.meta.name);
        }
        this.cancelDestination();
      },
      (error) => {
        if (this.isInline) {
          this._controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
        } else {
          this._controllerService.invokeRESTErrorToaster(Utility.CREATE_FAILED_SUMMARY, error);
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
}
