import { AfterViewInit, ViewChild, Component, OnDestroy, OnInit, ViewEncapsulation, EventEmitter, Input, Output, OnChanges, SimpleChanges, } from '@angular/core';
import { FormArray, AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';
import { BaseComponent } from '@components/base/base.component';
import { RolloutService } from '@app/services/generated/rollout.service';
import { ControllerService } from '@app/services/controller.service';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { ToolbarButton, ToolbarData } from '@app/models/frontend/shared/toolbar.interface';
import { IApiStatus, IRolloutRollout, RolloutRollout, RolloutRolloutSpec } from '@sdk/v1/models/generated/rollout';
import { IObjstoreObject, ObjstoreObject, IObjstoreObjectList, ObjstoreObjectList } from '@sdk/v1/models/generated/objstore';
import { RepeaterComponent, RepeaterData, ValueType } from 'web-app-framework';
import { LabelsRequirement_operator } from '@sdk/v1/models/generated/monitoring';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { required } from '@sdk/v1/utils/validators';
import { Utility } from '@common/Utility';
import { RolloutImageLabel, EnumRolloutOptions, RolloutImageOption } from '../index';
import { RolloutUtil} from '../RolloutUtil';



@Component({
  selector: 'app-newrollout',
  templateUrl: './newrollout.component.html',
  styleUrls: ['./newrollout.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NewrolloutComponent extends BaseComponent implements OnInit, OnDestroy, AfterViewInit, OnChanges {
  @ViewChild('orderConstraintsLabelRepeater') ocLabelRepeater: RepeaterComponent;

  newRollout: RolloutRollout;
  subscriptions = [];
  oldToolbarData: ToolbarData;
  strategyOptions: SelectItem[] = Utility.convertEnumToSelectItem(RolloutRolloutSpec.propInfo['strategy'].enum);
  upgradetypeOptions: SelectItem[] = Utility.convertEnumToSelectItem(RolloutRolloutSpec.propInfo['upgrade-type'].enum);
  rolloutImageOptions: RolloutImageOption[] = [];

  rolloutNicNodeTypes: SelectItem[] = Utility.convertEnumToSelectItem(EnumRolloutOptions);
  selectedRolloutNicNodeTypes: string = RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE;

  orderConstraintslabelData: RepeaterData[] = [];
  orderConstraintslabelFormArray = new FormArray([]);

  minDate: Date;

  @Input() isInline: boolean = false;
  @Input() selectedRolloutData: IRolloutRollout;
  @Input() existingRollouts: RolloutRollout[] = [];
  @Input() rolloutImages: IObjstoreObjectList;

  @Output() formClose: EventEmitter<any> = new EventEmitter();


  versiondescription: string = '';

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    matIcon: 'update'
  };

  constructor(private rolloutService: RolloutService, private objstoreService: ObjstoreService,
    protected controllerService: ControllerService) {
    super(controllerService);
  }

  getClassName(): string {
    return this.constructor.name;
  }


  ngOnInit() {
    // prepare for label-selector repeater
    this.orderConstraintslabelData = [
      {
        key: { label: 'text', value: 'text' },
        operators: Utility.convertEnumToSelectItem(LabelsRequirement_operator),
        fieldType: ValueType.inputField,
        valueType: ValueType.inputField
      }
    ];
    if (!this.newRollout) {
      // below ngChange() should initialize rolloutData.
      this.initRolloutData();
    }
    this.minDate = new Date();
  }

  initRolloutData() {
    if (!this.isInline) {  // create mode
      this.newRollout = new RolloutRollout();
      const today = new Date().getTime() + 300 * 1000; // VS-331 set default time. Now + 5 min
      this.newRollout.spec['scheduled-start-time'] = new Date(today);
      this.newRollout.setFormGroupValuesToBeModelValues();
      this.newRollout.$formGroup.get(['spec', 'suspend']).disable();
    } else {   // edit mode
      const myrollout: IRolloutRollout = Utility.getLodash().cloneDeep(this.selectedRolloutData);
      myrollout.spec['scheduled-start-time'] = new Date(myrollout.spec['scheduled-start-time']);
      this.newRollout = new RolloutRollout(myrollout);
      this.newRollout.$formGroup.get(['meta', 'name']).disable();
      this.newRollout.$formGroup.get(['spec', 'version']).disable(); // disable version until version options are available.
      this. selectedRolloutNicNodeTypes  = RolloutUtil.getRolloutNaplesVeniceType(this.newRollout);
    }
    // set validators. TODO: 2019-05-15 wait for server set validator in rollout.proto
    if (this.newRollout.$formGroup.validator) {
      this.newRollout.$formGroup.get(['meta', 'name']).setValidators([required, this.isRolloutNameValid(this.existingRollouts), this.newRollout.$formGroup.validator]);
    } else {
      this.newRollout.$formGroup.get(['meta', 'name']).setValidators([required, this.isRolloutNameValid(this.existingRollouts)]);
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).setValidators([required, this.isRolloutScheduleTimeValid()]);
      this.newRollout.$formGroup.get(['spec', 'version']).setValidators([required]);
    }
    this.newRollout.$formGroup.get(['spec', 'duration']).disable(); // TODO: Disable duration for this release 2019-05-02
  }


  ngAfterViewInit() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      this.oldToolbarData = this.controllerService.getToolbarData();
      const newToolbarData = this.controllerService.getToolbarData();
      newToolbarData.buttons = [
        {
          cssClass: 'global-button-primary newrollout-toolbar-button',
          text: 'SAVE ROLLOUT',
          callback: () => {
            this.addRollout();
          },
          computeClass: () => this.isAllInputsValidated() ? '' : 'global-button-disabled',
        },
        {
          cssClass: 'global-button-neutral newrollout-toolbar-button',
          text: 'CANCEL',
          callback: () => {
            this.cancelRollout();
          },
        },
      ];
      this.controllerService.setToolbarData(newToolbarData);
    }
  }

  isRolloutScheduleTimeValid(): ValidatorFn {
    const greateThan = 1;
    const now = new Date();
    return Utility.dateValidator( now, greateThan, 'schedule-time',  'Can not schedule rollout to the past' );
  }

  isRolloutNameValid(existingRollouts: RolloutRollout[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingRollouts, 'rollout-name');
  }

  isAllInputsValidated(): boolean {
    if (!this.newRollout || !this.newRollout.$formGroup) {
      return false;
    }
    if (Utility.isEmpty(this.newRollout.$formGroup.get(['meta', 'name']).value)) {
      return false;
    }
    if (Utility.isEmpty(this.newRollout.$formGroup.get(['spec', 'version']).value)) {
      return false;
    }
    if (Utility.isEmpty(this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value)) {
      return false;
    }
    const hasFormGroupError = Utility.getAllFormgroupErrors(this.newRollout.$formGroup);
    return hasFormGroupError === null;
  }

  ngOnDestroy() {
    this.subscriptions.forEach((subscription) => {
      subscription.unsubscribe();
    });
  }

  ngOnChanges(changes: SimpleChanges) {
    this.initRolloutData();
    this.populateRolloutVersions();
  }


  /**
   * Per back-end process, once user uploads a "bundle.tar" to Venice.
   *  https://venice:port/objstore/v1/tenant/default/images/objects result will contain
   *  [{
   *   "kind": "Object",
   *   "meta": {
   *     "name": "Bundle/0.10.0-130_img/metadata.json", // we are only interested in meta.name.endsWith('metadata.json')
   *     "generation-id": "",
   *     "labels": {
   *       "Releasedata": "",
   *       "Version": "0.10.0-130"   // get this version
   *    },
   * ...
   * ]
   */
  populateRolloutVersions() {
    const selectedVersion = this.newRollout.spec.version;
    this.rolloutImageOptions.length = 0;
    if (this.rolloutImages && this.rolloutImages.items) {
      this.rolloutImages.items.forEach(image => {
        if (image.meta.name.endsWith(RolloutUtil.ROLLOUT_METADATA_JSON)) {
          const imageLabel: RolloutImageLabel = image.meta.labels as RolloutImageLabel;
          if (imageLabel) {
            // rollout.spec.version is a string, so we set the selectedItem.value to string.
            const selectedItem: RolloutImageOption = {
              label: imageLabel.Version,
              value: imageLabel.Version,
              model: imageLabel // we need .model in getImageVersionData() api
            };
            this.rolloutImageOptions.push(selectedItem);
          }
        }
      });
      this.newRollout.$formGroup.get(['spec', 'version']).enable(); // enable version dropdown
    }
    this.computeVersionDescription(selectedVersion);
  }

  addRollout() {
    this._saveRollout();
  }

  updateRollout() {
    this._saveRollout();
  }

  /**
   * Build rollout JSON.
   * according to RolloutUtil.getRolloutNaplesVeniceType() rules
   */
  buildRollout(): IRolloutRollout {
    const rollout: IRolloutRollout = this.newRollout.getFormGroupValues();
    rollout.meta.name = (rollout.meta.name) ? rollout.meta.name : this.newRollout.meta.name;
    if (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_VENICE_ONLY) {
      rollout.spec['max-nic-failures-before-abort'] = null;
      rollout.spec['order-constraints'] = [];
      rollout.spec['smartnic-must-match-constraint'] = true;
      rollout.spec['smartnics-only'] = false;
    } else if (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_NAPLES_ONLY) {
      rollout.spec['smartnics-only'] = true;
      this.setSpecOrderConstrains(rollout);
      rollout.spec['max-parallel'] = null;
    } else if ( this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE) {
       rollout.spec['smartnics-only'] = false;
       this.setSpecOrderConstrains(rollout);
    }
     return rollout;
  }

  private setSpecOrderConstrains(rollout: IRolloutRollout) {
    if (rollout.spec['smartnic-must-match-constraint']) {
      rollout.spec['order-constraints'] = Utility.convertRepeaterValuesToSearchExpression(this.ocLabelRepeater); // Some Naples will be updated.
    } else {
      rollout.spec['order-constraints'] = []; // All Naples will be updated.
    }
  }

  _saveRollout() {
    const rollout: IRolloutRollout = this.buildRollout();
    let handler: Observable<{ body: IRolloutRollout | IApiStatus | Error, statusCode: number }>;

    /**  Disable this block as back-end does not support Add/Update(..) but using DoRollout(..)
    if (this.isInline) {
      // Using this.newRollout to get name, as the name is gone when we call getFormGroupValues
      // This is beacuse we disabled it in the form group to stop the user from editing it.
      // When you disable an angular control, in doesn't show up when you get the value of the group

       handler = this.rolloutService.UpdateRollout(this.newRollout.meta.name, rollout);
    } else {
       handler = this.rolloutService.AddRollout(rollout);
    }
    */
    handler = this.rolloutService.CreateRollout(rollout, null, false); // TODO: For now, add trimObject parameter as false because rollout.spec["scheduled-start-time"] got trimmed.
    const sub = handler.subscribe(
      (response) => {
        if (this.isInline) {
          this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Updated rollout ' + this.newRollout.meta.name);
        } else {
          this._controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Created rollout ' + rollout.meta.name);
        }
        this.cancelRollout();
      },
      (error) => {
        if (this.isInline) {
          this._controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
        } else {
          this._controllerService.invokeRESTErrorToaster(Utility.CREATE_FAILED_SUMMARY, error);
        }
      }
    );
    this.subscriptions.push(sub);
  }

  cancelRollout() {
    if (!this.isInline) {
      // Need to reset the toolbar that we changed
      this.setPreviousToolbar();
    }
    this.formClose.emit(false);
  }

  /**
   * Sets the previously saved toolbar buttons
   * They should have been saved in the ngOnInit when we are inline.
   */
  setPreviousToolbar() {
    if (this.oldToolbarData != null) {
      this._controllerService.setToolbarData(this.oldToolbarData);
    }
  }

  onVersionChange($event) {
    const selectedVersion = $event.value;
    this.computeVersionDescription(selectedVersion);
  }

  computeVersionDescription(selectedVersion: string) {
    const rolloutImageLabel = this.getImageVersionData(selectedVersion);
    this.versiondescription = rolloutImageLabel ? this.buildVersionDisplay(rolloutImageLabel) : '';
  }

  buildVersionDisplay(rolloutImageLabel: RolloutImageLabel): string {
    let result = '';
    if (rolloutImageLabel.Description) {
      result += rolloutImageLabel.Description;
    }
    if (rolloutImageLabel.Releasedate) {
      result += ' Release: ' + rolloutImageLabel.Releasedate;
    }
    if (rolloutImageLabel.Environment) {
      result += ' Env: ' + rolloutImageLabel.Environment;
    }
    return result;
  }

  getImageVersionData(version: string): RolloutImageLabel {
    const targets: RolloutImageOption[] = this.rolloutImageOptions.filter(selectItem => {
      return (selectItem.value === version);
    });
    return (targets && targets.length > 0) ? targets[0].model : null;
  }

  /**
   * This API serves html template
   */
  onNicNodeTypeChange($event) {
    this.selectedRolloutNicNodeTypes = $event.value;
  }

  /**
   * This API serves html template
   */
  onNICsConstraintsChange($event) {
    // debug. console.log(this.getClassName() + '.onNICsConstraintsChange()' + event);
  }

  /**
   * This API serves html template
   */
  isToShowNodeDiv(): boolean {
    return (this.selectedRolloutNicNodeTypes !== RolloutUtil.ROLLOUTTYPE_NAPLES_ONLY);
  }

  /**
   * This API serves html template
   */
  isToShowNicDiv(): boolean {
    return (this.selectedRolloutNicNodeTypes !== RolloutUtil.ROLLOUTTYPE_VENICE_ONLY);
  }

  /**
   * This API serves html template
   */
  isToShowNicOrderDiv(): boolean {
    return this.newRollout.$formGroup.get(['spec', 'smartnic-must-match-constraint']).value;
  }


}
