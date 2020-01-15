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



/**
 * NewhostComponent extends CreationForm
 * It enable adding and updating host object.  User must specify to use "ID" or "MAC" in UI-html
 * Internally, 'radioValue' holds the selected value (ID or MAC)
 *
 *  postNgInit() -> getPreSelectedDistributedServiceCardID() // when in edit mode, we compute the radio value from data
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
  radioValue: string = '';

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

    // gets the options for the radio buttons
    this.smartNICIDOptions = Object.keys((<any>this.newHostForm.get(['spec', 'dscs', 0])).controls);

    this.setValidators(this.newObject);
  }

  setValidators(newObject: ClusterHost) {
    newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewHostNameValid(this.existingObjects)]);
    newObject.$formGroup.get(['spec', 'dscs', 0, 'mac-address']).setValidators([
      patternValidator(Utility.MACADDRESS_REGEX, NewhostComponent.MACADDRESS_MESSAGE)]);
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

  getPreSelectedDSCID() {
    // sets radio when editing

    if (this.newObject.spec['dscs'].length !== 0) {
      const clusterDSCID: ClusterDistributedServiceCardID = this.newObject.spec['dscs'][0];

      if (clusterDSCID[NewhostComponent.KEYS_ID] !== null) {
        this.radioValue = NewhostComponent.KEYS_ID;
      }
      if (clusterDSCID[NewhostComponent.KEYS_MACADDRESS] !== null) {
        this.radioValue = NewhostComponent.KEYS_MACADDRESS;
      }
      if (clusterDSCID[NewhostComponent.KEYS_ID] === null && clusterDSCID[NewhostComponent.KEYS_MACADDRESS] === null) {
        this.radioValue = '';
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
    smartNICIDs.insert(0, new ClusterDistributedServiceCardID().$formGroup);
  }

  onRadioButtonChange($event) {
    // changes value of radio to the one the user has selected
    this.radioValue = $event.value;
    if (this.radioValue === 'id') {
      this.newHostForm.get(['spec', 'dscs', 0, 'mac-address']).setValue(null);
    } else {
      this.newHostForm.get(['spec', 'dscs', 0, 'id']).setValue(null);
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
      if (!Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', 0, this.radioValue]).value)
        && this.newHostForm.get(['spec', 'dscs', 0, this.radioValue]).valid) {
        return true;
      }
      if (!this.newHostForm.get(['spec', 'dscs', 0, 'mac-address']).valid) {
        this.createButtonTooltip = NewhostComponent.MACADDRESS_MESSAGE;
        return false;
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
  isFieldEmpty(field: AbstractControl): boolean {
    return Utility.isEmpty(field.value);
  }
  getTooltip() {
    return this.createButtonTooltip;
  }
  getFieldTooltip() {
    if (Utility.isEmpty(this.newHostForm.get(['spec', 'dscs', 0, this.radioValue]).value)) {
     return `${this.radioValue} field is empty`;
  }
  if (!this.newHostForm.get(['spec', 'dscs', 0, 'mac-address']).valid) {
    return NewhostComponent.MACADDRESS_MESSAGE;
  }
}
}
