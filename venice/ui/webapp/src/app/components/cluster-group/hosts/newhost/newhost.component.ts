import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, OnDestroy, ViewChild, ChangeDetectorRef} from '@angular/core';
import { Animations } from '@app/animations';
import { BaseComponent } from '@app/components/base/base.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ClusterHost, IClusterHost} from '@sdk/v1/models/generated/cluster/cluster-host.model';
import {ClusterSmartNICID} from '@sdk/v1/models/generated/cluster/cluster-smart-nicid.model';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { Utility } from '@app/common/Utility';
import { required, patternValidator } from '@sdk/v1/utils/validators';
import { FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { IApiStatus } from '@sdk/v1/models/generated/cluster';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';

/**
 * NewhostComponent extends CreationForm
 * It enable adding and updating host object.  User must specify to use "ID" or "MAC" in UI-html
 * Internally, 'radioValue' holds the selected value (ID or MAC)
 *
 *  postNgInit() -> getPreSelectedSmartNICID() // when in edit mode, we compute the radio value from data
 *
 * In createObjct() and updateObject(), we clean up data using clearOtherRadios()
 */
@Component({
  selector: 'app-newhost',
  templateUrl: './newhost.component.html',
  styleUrls: ['./newhost.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
  export class NewhostComponent  extends CreationForm<IClusterHost, ClusterHost> implements OnInit, AfterViewInit, OnDestroy {

  public static KEYS_ID = 'id';
  public static KEYS_MACADDRESS = 'mac-address';

  public static MACADDRESS_REGEX: string = '^([0-9a-fA-F]{4}[.]){2}([0-9a-fA-F]{4})$';
  public static MACADDRESS_MESSAGE: string = 'MAC address must be of the format aaaa.bbbb.cccc';

  @Input() isInline: boolean = false;
  @Input() existingObjects: IClusterHost[] = [];
  @Input() objectData: IClusterHost;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  newHostForm: FormGroup;

  smartNICIDs: any;

  smartNICIDOptions: string[] = [];

  // This property keep track of user input selection (ID or MAC)
  radioValue: string = '';

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _clusterService: ClusterService,
  ) {
    super(_controllerService, uiconfigsService, ClusterHost);
  }

  postNgInit() {
    if (this.objectData != null) {
      // in edit mode.  We compute the radio (ID/MAC) value
      this.getPreSelectedSmartNICID();
    }

    this.newHostForm = this.newObject.$formGroup;
    const smartNICIDs: any = this.newHostForm.get(['spec', 'smart-nics']);

    if (smartNICIDs.controls.length === 0) {
      this.addSmartNICID();
    }

    if (this.isInline) {
      // disable name field
      this.newObject.$formGroup.get(['meta', 'name']).disable();
    }

    this.smartNICIDs = (<any>this.newHostForm.get(['spec', 'smart-nics'])).controls;

    // gets the options for the radio buttons
    this.smartNICIDOptions = Object.keys((<any>this.newHostForm.get(['spec', 'smart-nics', 0])).controls);

    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewHostNameValid(this.existingObjects) ]);

    this.newObject.$formGroup.get(['spec', 'smart-nics', 0, 'mac-address']).setValidators([
      patternValidator(NewhostComponent.MACADDRESS_REGEX, NewhostComponent.MACADDRESS_MESSAGE) ]);

  }


  getPreSelectedSmartNICID() {
    // sets radio when editing

    if (this.newObject.spec['smart-nics'].length !== 0) {
      const clusterSmartNICID: ClusterSmartNICID = this.newObject.spec['smart-nics'][0];

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

  computeButtonClass() {
    if (Utility.isEmpty(this.newObject.$formGroup.get('meta.name').value)) {
      return 'global-button-disabled';
    }
    if ( this.newObject.$formGroup.get('meta.name').status === 'VALID' &&
    this.isFormValid()) {
      return '';
    } else {
      return 'global-button-disabled';
    }
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

  /**
   * User can flip between id and mac, we want to clear the non-selected field.
   * Say, the radio value is 'id', we remove the 'mac-address' value
   */
  clearOtherRadios(): IClusterHost {
    const host: IClusterHost = this.newObject.getFormGroupValues();
    for (let i = 0; i < host.spec['smart-nics'].length; i++) {
      const config = host.spec['smart-nics'][i];
      const keys = Object.keys(config);
      for (let j = 0; j < keys.length; j++) {
        const key = keys[j];
        if (this.radioValue !== key) {
          delete config[key];
        }
      }
    }
    return host;
  }

  isFormValid(): boolean {
    // checks that the ADD NAPLES BY field is filled out
    if (this.radioValue !== '') {
      if (!Utility.isEmpty(this.newHostForm.get(['spec', 'smart-nics', 0, this.radioValue]).value)
      && this.newHostForm.get(['spec', 'smart-nics', 0, this.radioValue]).valid) {
        return true;
      }
    }
    return false;
  }

  isNewHostNameValid(existingObjects: IClusterHost[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newHost-name');
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setToolbar(): void {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary host-button host-button-SAVE',
          text: 'CREATE HOST',
          callback: () => { this.saveObject(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-neutral host-button host-button-CANCEL',
          text: 'CANCEL',
          callback: () => {
            this.cancelObject(); // this.cancelHost();
           }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }

  /**
   * override super api
   * We make up the JSON object
   */
  createObject(newObject: IClusterHost): Observable<{ body: IClusterHost | IApiStatus | Error; statusCode: number; }> {
    const host: IClusterHost = this.clearOtherRadios();
    return this._clusterService.AddHost(host);
  }

  /**
   * override super api
   * We make up the JSON object
   */
  updateObject(newObject: IClusterHost, oldObject: IClusterHost): Observable<{ body: IClusterHost | IApiStatus | Error; statusCode: number; }> {
    const host: IClusterHost = this.clearOtherRadios();
    return this._clusterService.UpdateHost(oldObject.meta.name, host);
  }

  generateCreateSuccessMsg(object: IClusterHost): string {
    return 'Created host ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IClusterHost): string {
    return 'Updated host ' + object.meta.name;
  }

  getObjectValues(): IClusterHost {
    return  this.newObject.getFormGroupValues();
  }
}
