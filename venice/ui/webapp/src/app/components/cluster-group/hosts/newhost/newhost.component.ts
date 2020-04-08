import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, OnDestroy, ViewChild, ChangeDetectorRef } from '@angular/core';
import { Animations } from '@app/animations';
import { BaseComponent } from '@app/components/base/base.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ClusterHost, IClusterHost } from '@sdk/v1/models/generated/cluster/cluster-host.model';
import { ClusterDistributedServiceCardID } from '@sdk/v1/models/generated/cluster/cluster-distributed-service-card-id.model';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { Utility } from '@app/common/Utility';
import { required, patternValidator } from '@sdk/v1/utils/validators';
import { FormGroup, FormArray, ValidatorFn, AbstractControl } from '@angular/forms';
import { IApiStatus } from '@sdk/v1/models/generated/cluster';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { TableCol } from '@app/components/shared/tableviewedit';
import { ClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { IClusterAutoMsgDistributedServiceCardWatchHelper } from '@sdk/v1/models/generated/cluster';
import { StagingService } from '@app/services/generated/staging.service';
import { StagingBuffer, StagingCommitAction } from '@sdk/v1/models/generated/staging';
import { map, switchMap, tap, catchError, buffer } from 'rxjs/operators';
import { Observable, forkJoin, throwError } from 'rxjs';


export interface DSCIDuiModel {
  isUseID: boolean;
}
/**
 * NewhostComponent extends CreationForm
 * It enable adding and updating host object.  User must specify to use "ID" or "MAC" in UI-html
 * Internally, 'radioValue' holds the selected value (ID or MAC)
 *
 *  postNgInit() -> getPreSelectedDistributedServiceCardID() // when in edit mode, we compute the radio value from data
 *
 * In createObjct() and updateObject(), we clean up data using clearOtherRadios()
 */

/*
By 2020-03-24, one host can host tow DSCs.  We can mix DSC id or DSC mac-address as shown below. We can use either id or mac-address, but not both
We use this.radioValues[i] to keep track wheterh the i-th DSC is using id or address.
Host{
	...
  "spec": {
         "dscs": [
           {
             "mac-address": “00ae.cd00.0009”  // DSC-A’s mac
           },
            {
             “id": “DSC-B  “  // using DSC-B’s id
           }
         ]
       }
*/
@Component({
  selector: 'app-newhost',
  templateUrl: './newhost.component.html',
  styleUrls: ['./newhost.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewhostComponent extends CreationForm<IClusterHost, ClusterHost> implements OnInit, AfterViewInit, OnDestroy {

  public static KEYS_ID = 'id';
  public static KEYS_MACADDRESS = 'mac-address';

  public static MACADDRESS_REGEX: string = '^([0-9a-fA-F]{4}[.]){2}([0-9a-fA-F]{4})$';
  public static MACADDRESS_MESSAGE: string = 'MAC address must be of the format aaaa.bbbb.cccc';

  @Input() isInline: boolean = false;
  @Input() existingObjects: IClusterHost[] = [];
  @Input() objectData: IClusterHost;
  @Input() dataObjects: ClusterDistributedServiceCard[] = [];
  @Input() notAdmittedCount: number = 0;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  createButtonTooltip: string = '';
  newHostForm: FormGroup;
  smartNICIDs: any;
  smartNICIDOptions: string[] = [];

  // This property keep track of user input selection (ID or MAC)
  radioValues: string[] = [];

  cols: TableCol[] = [
    { field: 'meta.name', header: 'DSC Name', class: '', sortable: true, width: 20 },
    { field: 'meta.mac-address', header: 'Mac Address', class: '', sortable: true, width: 20 },
    { field: 'add_host', header: 'Add Host', class: '', sortable: true, width: 60 },
  ];

  naplesEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;
  naples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  objectMap: any = {};

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected _clusterService: ClusterService,
    protected stagingService: StagingService
  ) {
    super(_controllerService, uiconfigsService, ClusterHost);
  }

  postNgInit() {
    if (!this.isInline) {
      this.processNaples();
    }
    if (this.objectData != null) {
      // in edit mode.  We compute the radio (ID/MAC) value
      this.getPreSelectedDSCID();
    }

    this.newHostForm = this.newObject.$formGroup;
    const smartNICIDs: any = this.newHostForm.get(['spec', 'dscs']);

    if (smartNICIDs.controls.length === 0) {
      this.addDSCID(this.newObject);
    }

    if (this.isInline) {
      // disable name field
      this.newObject.$formGroup.get(['meta', 'name']).disable();
    }

    this.smartNICIDs = (<any>this.newHostForm.get(['spec', 'dscs'])).controls;

    // gets the options for the radio button
    this.smartNICIDOptions = Object.keys((<any>this.newHostForm.get(['spec', 'dscs', 0])).controls);

    this.setValidators(this.newObject);
  }

  setValidators(newObject: ClusterHost) {
    newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewHostNameValid(this.existingObjects)]);
    for (let i = 0; i < this.radioValues.length; i++) {
      this.setDSCValidator(newObject, 0);
    }
  }

  shouldRadioButtonBeChecked(smartNICIDOption, index): boolean {
    const control  = this.newObject.$formGroup.get(['spec', 'dscs', index, smartNICIDOption]);
    return (control && control.value);
  }

  private setDSCValidator(newObject: ClusterHost, index: number) {
    newObject.$formGroup.get(['spec', 'dscs', index, 'mac-address']).setValidators([
      patternValidator(Utility.MACADDRESS_REGEX, NewhostComponent.MACADDRESS_MESSAGE)
    ]);
  }

  processNaples() {
    this.objectMap = {};
    for (const dsc of this.dataObjects) {
      if (!dsc.status.host) {
        const newHost = new ClusterHost();
        this.addDSCID(newHost);
        this.setValidators(newHost);
        newHost.$formGroup.get(['spec', 'dscs', 0, 'mac-address']).setValue(dsc.status['primary-mac']);
        this.objectMap[dsc.meta.name] = newHost;
      }
    }
  }

  getFormGroup(rowData: ClusterHost): FormGroup {
    return this.objectMap[rowData.meta.name].$formGroup;
  }

  /**
   *  Sets radio when editing
   *
   *  TODO: 2020-03-24, once we support host update, fully test the API
   */
  getPreSelectedDSCID() {
    if (this.newObject.spec['dscs'].length !== 0) {
      this.radioValues = []; // clean up radioValues array
      for (let i = 0; i < this.newObject.spec['dscs'].length; i++) {
        const clusterDSCID: ClusterDistributedServiceCardID = this.newObject.spec['dscs'][i];

        if (clusterDSCID[NewhostComponent.KEYS_ID] !== null) {
          this.radioValues[i] = NewhostComponent.KEYS_ID;
        }
        if (clusterDSCID[NewhostComponent.KEYS_MACADDRESS] !== null) {
          this.radioValues[i] = NewhostComponent.KEYS_MACADDRESS;
        }
        if (clusterDSCID[NewhostComponent.KEYS_ID] === null && clusterDSCID[NewhostComponent.KEYS_MACADDRESS] === null) {
          this.radioValues[i] = '';
        }
      }
    }

  }

  computeButtonClass() {
    let enable = false;
    if (this.notAdmittedCount !== 0) {
      for (const key of Object.keys(this.objectMap)) {
        const i = this.objectMap[key].$formGroup;
        if (!Utility.isEmpty(i.get('meta.name').value) && i.get('meta.name').status === 'VALID') {
          enable = true;
        }
      }
    } else {
      if (Utility.isEmpty(this.newObject.$formGroup.get('meta.name').value)) {
        return 'global-button-disabled';
      }
      if (this.newObject.$formGroup.get('meta.name').status === 'VALID' && this.isFormValid()) {
        enable = true;
      }
    }

    if (enable) {
      return '';
    }
    return 'global-button-disabled';
  }

  addDSCID(newObject: ClusterHost) {
    // updates the form
    const smartNICIDs = newObject.$formGroup.get(['spec', 'dscs']) as FormArray;
    const clusterDistributedServiceCardID: ClusterDistributedServiceCardID = new ClusterDistributedServiceCardID();
    smartNICIDs.insert(0, clusterDistributedServiceCardID.$formGroup);
    this.radioValues.push('');
  }

  onRadioButtonChange($event, index: number) {
    // changes value of radio to the one the user has selected
    this.radioValues[index] = $event.value;
    if (this.radioValues[index] === 'id') {
      this.newHostForm.get(['spec', 'dscs', index, 'mac-address']).setValue(null);
    } else {
      this.newHostForm.get(['spec', 'dscs', index, 'id']).setValue(null);
    }
  }




  /**
   * User can flip between id and mac, we want to clear the non-selected field.
   * Say, the radio value is 'id', we remove the 'mac-address' value
   */
  clearOtherRadios(): IClusterHost {
    const host: IClusterHost = this.newObject.getFormGroupValues();
    for (let i = 0; i < host.spec['dscs'].length; i++) {
      const config = host.spec['dscs'][i];
      const keys = Object.keys(config);
      for (let j = 0; j < keys.length; j++) {
        const key = keys[j];
        if (this.radioValues[i] !== key) {
          delete config[key];
        }
      }
    }
    return host;
  }

  isFormValid(): boolean {
    if (!this.radioValues.length) {
      this.createButtonTooltip = 'Please pick either id or mac-address';
      return false;
    }
    // checks that the ADD NAPLES BY field is filled out
    for (let i = 0; i < this.radioValues.length; i++) {
      if (this.radioValues[i] !== '') {
        // both id and mac-address are empty
        if (Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', i, 'id']).value) &&
          Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', i, 'mac-address']).value)) {
          this.createButtonTooltip = 'Need either id or mac-address input in box ' + (i + 1);
          return false;
        }
        // both id and mac-address are not empty.
        if (!Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', i, 'id']).value) &&
          !Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', i, 'mac-address']).value)) {
          this.createButtonTooltip = 'Can not have both id and mac-address input in box ' + (i + 1);
          return false;
        }

        // if "id" field is not empty or has invalid input, return false
        if (!Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', i, 'id']).value)
          && !this.newHostForm.get(['spec', 'dscs', i, this.radioValues[i]]).valid) {
          this.createButtonTooltip = 'Input id field is not valid in box ' + (i + 1);
          return false;
        }
        // if "mac" field is not empty and has invalid input, return false
        if (!Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', i, 'id']).value)
          && !this.newHostForm.get(['spec', 'dscs', i, 'mac-address']).valid) {
          this.createButtonTooltip = 'Box ' + (i + 1) + ' ' + NewhostComponent.MACADDRESS_MESSAGE;
          return false;
        }
      } else {
        this.createButtonTooltip = 'Pick either id or mac-address input in  box ' + (i + 1);
        return false;
      }
    }
    if (!this.newObject.$formGroup.valid) {
      this.createButtonTooltip = 'Input data is not valid';
      return false;
    }
    return true;
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
          text: 'CREATE HOSTS',
          genTooltip: () => this.getTooltip(),
          callback: () => { this.saveObjects(); },
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
    // const host: IClusterHost = this.clearOtherRadios();
    return this._clusterService.AddHost(newObject);
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
    return this.newObject.getFormGroupValues();
  }

  commitStagingBuffer(buffername: string): Observable<any> {
    const commitBufferBody: StagingCommitAction = Utility.buildCommitBufferCommit(buffername);
    return this.stagingService.Commit(buffername, commitBufferBody);
  }

  createStagingBuffer(): Observable<any> {
    const stagingBuffer: StagingBuffer = Utility.buildCommitBuffer();
    return this.stagingService.AddBuffer(stagingBuffer);
  }

  deleteStagingBuffer(buffername: string, reason: string, isToshowToaster: boolean = true) {
    if (buffername == null) {
      return;
    }

    // Whenever, we have to call delete buffer, there must be error occurred. We print out the buffer detail here.
    this.stagingService.GetBuffer(buffername).subscribe((res) => {
      console.error(this.getClassName() + '.deleteStagingBuffer() API. Invoke GetBuffer():', res);
    });
    this.stagingService.DeleteBuffer(buffername).subscribe(
      response => {
        if (isToshowToaster) {
          this._controllerService.invokeSuccessToaster('Successfully deleted buffer', 'Deleted Buffer ' + buffername + '\n' + reason);
        }
      },
      this._controllerService.restErrorHandler('Delete Staging Buffer Failed')
    );
  }

  saveBatchMode() {
    let createdBuffer: StagingBuffer = null;
    let bufferName: string;
    const observables: Observable<any>[] = [];
    this.createStagingBuffer()
      .pipe(
        switchMap(responseBuffer => {
          createdBuffer = responseBuffer.body as StagingBuffer;
          bufferName = createdBuffer.meta.name;
          for (const key of Object.keys(this.objectMap)) {
            const object = this.objectMap[key];
            const policy = object.getFormGroupValues();

            const fg = object.$formGroup;
            if (Utility.isEmpty(fg.get('meta.name').value) || fg.get('meta.name').status !== 'VALID') {
              continue;
            }

            const observable = this._clusterService.AddHost(policy, bufferName);
            if (observable == null) {
              continue;
            }
            observables.push(observable);
          }

          if (observables.length > 0) {
            return forkJoin(observables)
              .pipe(
                switchMap(results => {
                  const isAllOK = Utility.isForkjoinResultAllOK(results);
                  if (isAllOK) {
                    return this.commitStagingBuffer(bufferName);
                  } else {
                    const error = Utility.joinErrors(results);
                    return throwError(error);
                  }
                })
              );
          }
        })
      ).subscribe((responseCommitBuffer) => {
        this._controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Successfully added Hosts');
        this.deleteStagingBuffer(bufferName, 'Batch add successful', false);
        this.cancelObject();
      },
        (error) => {
          // any error in (A) (B) or (C), error will land here
          this._controllerService.invokeRESTErrorToaster(Utility.CREATE_FAILED_SUMMARY, error);
          this.deleteStagingBuffer(bufferName, 'Failed to add hosts', false);
        }
      );
  }

  saveSingleAddMode() {
    this.saveObject();
  }

  saveObjects() {
    if (this.notAdmittedCount === 0) {
      this.saveSingleAddMode();
    } else {
      this.saveBatchMode();
    }
  }
  getTooltip() {
    return this.createButtonTooltip;
  }

  getFieldTooltip(): string  {
    // TODO: 2020-03-24 host update is not supported. This api is not fully tested.
    const dscLen = this.newHostForm.get(['spec', 'dscs'])['length']; // spec.dsc is a formarray
    for (let i = 0; i < dscLen ; i++) {
      for (let j = 0; j < this.radioValues.length; j++) {
        const control = this.newHostForm.get(['spec', 'dscs', i,  this.radioValues[j]]);
        if (control && Utility.isEmpty(control.value)) {
          return `${this.radioValues[j]} field is empty`;
        }
        if (!this.newHostForm.get(['spec', 'dscs', i, 'mac-address']).valid) {
          return NewhostComponent.MACADDRESS_MESSAGE;
        }
      }
    }
  }

  /**
   * When user call + to add a new  DSC
   */
  addDSC() {
    const tempTargets = this.newHostForm.get(['spec', 'dscs']) as FormArray;
    const newFormGroup: FormGroup = new ClusterDistributedServiceCardID().$formGroup;
    tempTargets.push(newFormGroup);
    this.radioValues.push('');
    this.setDSCValidator(this.newObject, this.radioValues.length - 1);
  }

  /**
   * When uesr click delete icon to remvoe a DSC
   * @param index
   */
  removeDSC(index) {
    const tempTargets = this.newHostForm.get(['spec', 'dscs']) as FormArray;
    if (tempTargets.length > 1) {
      tempTargets.removeAt(index);
      this.radioValues.splice(index, 1);
    }
  }
}
