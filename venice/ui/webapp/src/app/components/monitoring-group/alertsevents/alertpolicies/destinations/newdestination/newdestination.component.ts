import { Component, OnInit, ViewEncapsulation, Input, AfterViewInit, Output, EventEmitter } from '@angular/core';
import { SelectItem } from 'primeng/primeng';
import { FormGroup, FormControl, FormArray, Validators } from '@angular/forms';
import { Animations } from '@app/animations';
import { IMonitoringAlertDestination, MonitoringAlertDestination, MonitoringSNMPTrapServer, MonitoringAuthConfig, MonitoringPrivacyConfig, IApiStatus } from '@sdk/v1/models/generated/monitoring';
import { ControllerService } from '@app/services/controller.service';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ErrorStateMatcher } from '@angular/material';
import { Utility } from '@app/common/Utility';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-newdestination',
  templateUrl: './newdestination.component.html',
  styleUrls: ['./newdestination.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewdestinationComponent implements OnInit, AfterViewInit {

  newDestination: MonitoringAlertDestination;

  @Input() isInline: boolean = false;
  @Input() destinationData: IMonitoringAlertDestination;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  versionOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringSNMPTrapServer.enumProperties['version'].enum);

  authAlgoOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringAuthConfig.enumProperties['algo'].enum);

  encryptAlgoOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringPrivacyConfig.enumProperties['algo'].enum);

  oldButtons: ToolbarButton[] = [];

  errorChecker = new ErrorStateMatcher();

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService
  ) { }

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
    // Check if there is a snmp trap server config, 
    // if not, we need to add one so that it shows in the form
    const snmpArray = this.newDestination.$formGroup.get(['spec', 'snmp-trap-servers']) as FormArray;
    if (snmpArray.length == 0) {
      snmpArray.insert(0, new MonitoringSNMPTrapServer().$formGroup);
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
          callback: () => { this.saveDestination() },
          computeClass: () => { return this.computeButtonClass() }
        },
        {
          cssClass: 'global-button-neutral eventalertpolicies-button',
          text: 'CANCEL',
          callback: () => { this.cancelDestination() }
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
    if (this.isInline) {
      handler = this._monitoringService.UpdateAlertDestination(this.newDestination.meta.name, this.newDestination);
    } else {
      handler = this._monitoringService.AddAlertDestination(this.newDestination)
    }
    handler.subscribe((response) => {
      const status = response.statusCode;
      if (status === 200) {
        if (!this.isInline) {
          // Need to reset the toolbar that we changed
          this.setPreviousToolbar();
        }
        this.formClose.emit();
      } else {
        console.error(response.body);
      }
    })
  };

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
