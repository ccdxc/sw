import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild, ChangeDetectorRef} from '@angular/core';
import { Animations } from '@app/animations';
import { BaseComponent } from '@app/components/base/base.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ClusterHost, IClusterHost} from '@sdk/v1/models/generated/cluster/cluster-host.model';
import {ClusterSmartNICID} from '@sdk/v1/models/generated/cluster/cluster-smart-nicid.model';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { SyslogComponent } from '@app/components/monitoring-group/syslog/syslog.component';
import { Utility } from '@app/common/Utility';
import { required, patternValidator } from '@sdk/v1/utils/validators';
import { FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { TargetLocator } from 'selenium-webdriver';
import { IApiStatus } from '@sdk/v1/models/generated/cluster';

@Component({
  selector: 'app-newhost',
  templateUrl: './newhost.component.html',
  styleUrls: ['./newhost.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewhostComponent extends BaseComponent implements OnInit, AfterViewInit {

  public static KEYS_ID = 'id';
  public static KEYS_MACADDRESS = 'mac-address';

  public static MACADDRESS_REGEX: string = '^([0-9a-fA-F]{4}[.]){2}([0-9a-fA-F]{4})$';
  public static MACADDRESS_MESSAGE: string = 'MAC address must be of the format aaaa.bbbb.cccc';

  newHost: ClusterHost;

  @Input() isInline: boolean = false;
  @Input() existingObjects: IClusterHost[] = [];
  @Input() hostData: IClusterHost;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  newHostForm: FormGroup;

  smartNICIDs: any;

  oldButtons: ToolbarButton[] = [];

  smartNICIDOptions: string[] = [];

  radioValue: string = '';

  constructor(protected _controllerService: ControllerService,
    protected _clusterService: ClusterService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    if (this.hostData != null) {
      this.newHost = new ClusterHost(this.hostData);
      this.getPreSelectedSmartNICID();
    } else {
      this.newHost = new ClusterHost();
    }

    this.newHostForm = this.newHost.$formGroup;
    const smartNICIDs: any = this.newHostForm.get(['spec', 'smart-nics']);

    if (smartNICIDs.controls.length === 0) {
      this.addSmartNICID();
    }

    if (this.isInline) {
      // disable name field
      this.newHost.$formGroup.get(['meta', 'name']).disable();
    }

    this.smartNICIDs = (<any>this.newHostForm.get(['spec', 'smart-nics'])).controls;

    // gets the options for the radio buttons
    this.smartNICIDOptions = Object.keys((<any>this.newHostForm.get(['spec', 'smart-nics', 0])).controls);

    this.newHost.$formGroup.get(['meta', 'name']).setValidators([
      this.newHost.$formGroup.get(['meta', 'name']).validator,
      this.isNewHostNameValid(this.existingObjects) ]);

    this.newHost.$formGroup.get(['spec', 'smart-nics', 0, 'mac-address']).setValidators([
      patternValidator(NewhostComponent.MACADDRESS_REGEX, NewhostComponent.MACADDRESS_MESSAGE) ]);

  }

  ngAfterViewInit() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary host-button host-button-SAVE',
          text: 'CREATE HOST',
          callback: () => { this.saveHost(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-neutral host-button host-button-CANCEL',
          text: 'CANCEL',
          callback: () => { this.cancelHost(); }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }

  getPreSelectedSmartNICID() {
    // sets radio when editing

    if (this.newHost.spec['smart-nics'].length !== 0) {
      const clusterSmartNICID: ClusterSmartNICID = this.newHost.spec['smart-nics'][0];

      if ( clusterSmartNICID[NewhostComponent.KEYS_ID] !== null) {
        this.radioValue = NewhostComponent.KEYS_ID;
      }
      if ( clusterSmartNICID[NewhostComponent.KEYS_MACADDRESS] !== null) {
        this.radioValue =  NewhostComponent.KEYS_MACADDRESS;
      }
      if ( clusterSmartNICID[NewhostComponent.KEYS_ID] === null && clusterSmartNICID[NewhostComponent.KEYS_MACADDRESS] === null) {
        this.radioValue = '';
      }
    }

  }

  setPreviousToolbar() {
    if (this.oldButtons != null) {
      const currToolbar = this._controllerService.getToolbarData();
      currToolbar.buttons = this.oldButtons;
      this._controllerService.setToolbarData(currToolbar);
    }
  }

  computeButtonClass() {
    if (Utility.isEmpty(this.newHost.$formGroup.get('meta.name').value)) {
      return 'global-button-disabled';
    }
    if ( this.newHost.$formGroup.get('meta.name').status === 'VALID' &&
    this.isValidForm()) {
      return '';
    } else {
      return 'global-button-disabled';
    }
  }

  cancelHost() {
    if (!this.isInline) {
      // Need to reset the toolbar that we changed
      this.setPreviousToolbar();
    }
    this.formClose.emit();
  }

  addSmartNICID() {
    // updates the form
    const smartNICIDs = this.newHostForm.get(['spec', 'smart-nics']) as FormArray;
    smartNICIDs.insert(0, new ClusterSmartNICID().$formGroup);
  }

  onRadioButtonChange($event) {
    // changes value of radio to the one the user has selected
    this.radioValue = $event.value;
  }

  clearOtherRadios() {
    // clears the other nonselected radio values if their forms have values in them
    const smartNICID: FormGroup = <FormGroup>this.newHostForm.get(['spec', 'smart-nics', 0]);
    for (let i = 0; i < this.smartNICIDOptions.length; i++) {
      const j: string = this.smartNICIDOptions[i];
      if (j !== this.radioValue) {
        smartNICID.controls[j].setValue(null);
      }
    }
  }

  isValidForm() {
    // checks that the ADD NAPLES BY field is filled out
    if (this.radioValue !== '') {
      if (!Utility.isEmpty(this.newHostForm.get(['spec', 'smart-nics', 0, this.radioValue]).value)
      && this.newHostForm.get(['spec', 'smart-nics', 0, this.radioValue]).valid) {
        return true;
      }
    }
    return false;
  }

  saveHost() {
    // Submit to server
    this.clearOtherRadios();
    const host: IClusterHost = this.newHost.getFormGroupValues();
    let handler: Observable<{ body: IClusterHost | IApiStatus | Error, statusCode: number }>;


    if (this.isInline) {
      handler = this._clusterService.UpdateHost(this.newHost.meta.name, host);
    } else {
      handler = this._clusterService.AddHost(host);
    }

    handler.subscribe(
      (response) => {
        if (this.isInline) {
          this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Updated host ' + this.newHost.meta.name);
        } else {
          this._controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Created host ' + host.meta.name);
        }
        this.cancelHost();
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

  isNewHostNameValid(existingObjects: IClusterHost[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newHost-name');
  }





}
