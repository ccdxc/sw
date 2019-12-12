import { AfterViewInit, ChangeDetectorRef, Component, EventEmitter, Input, IterableDiffer, OnChanges, OnDestroy, OnInit, Output, Renderer2, SimpleChanges, TemplateRef, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormArray, ValidatorFn, FormGroup } from '@angular/forms';
import { BaseComponent } from '@components/base/base.component';
import { RolloutService } from '@app/services/generated/rollout.service';
import { ControllerService } from '@app/services/controller.service';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { ToolbarData } from '@app/models/frontend/shared/toolbar.interface';
import { IApiStatus, IRolloutRollout, RolloutRollout, RolloutRolloutSpec } from '@sdk/v1/models/generated/rollout';
import { IObjstoreObjectList } from '@sdk/v1/models/generated/objstore';
import { RepeaterData, ValueType, RepeaterItem } from 'web-app-framework';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { required } from '@sdk/v1/utils/validators';
import { Utility } from '@common/Utility';
import { Checkbox } from 'primeng/checkbox';
import { RolloutImageLabel, EnumRolloutOptions, RolloutImageOption } from '../index';
import { RolloutUtil } from '../RolloutUtil';
import { SearchExpression } from '@app/components/search/index.ts';
import { LabelsSelector } from '@sdk/v1/models/generated/security';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { SearchService } from '@app/services/generated/search.service';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';

export class RolloutOrder {
  id: string;
  data: FormGroup;
  orderSummary: string = '';
  matchedDistributedServiceCards: string[] = [];
  inEdit: boolean = false;
  duplicatesWith: number[] = [];
  incomplete: boolean = true;
}

/**
 * Description:

  When Naples are assigned labels we can specify a sequence for the rollout to progress in.
  This means that the 1st order group will be updated before the 2nd and so on.
  The UI allows the user to select NICs for group with the help of label selectors.

  When one or more labels are selected for group the updated label count is shown on the order HTML element.
  User can choose to add multiple such group though only one can be edited at one time.

  The 'Save Rollout' button is disabled until isAllInputsValidated() return true.

  Everytime a change is made, we built the duplicateMatchMap.
  It is a mapping from nic.meta.name to list of orderindices
  We iterate over all orders (and their matches NICs) and built the map.
  When we see that a matched NIC already has order(s) in the map, we show the info icon to notify the user.

 * Key functions:
  onInit() -> fetch all Naples
  setSpecOrderConstrains() - generates the order constraints
  buildRollout() - built the rollout object based on the form
  saveRollout() - call the rollout create api with the built rollout object
  getRuleResults() - checks the naplesRuleMap to find NICs matching the given labels
  repeaterValueChange(index) - updates the order summary and match count, when the repeater value changes
  isAllInputsValidated()

 */

@Component({
  selector: 'app-newrollout',
  templateUrl: './newrollout.component.html',
  styleUrls: ['./newrollout.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NewrolloutComponent extends BaseComponent implements OnInit, OnDestroy, AfterViewInit, OnChanges {
  @ViewChild('checkboxSetDuration') checkboxSetDuration: Checkbox;
  @Input() existingObjects: IRolloutRollout[] = [];

  orders: RolloutOrder[] = [];
  repeaterList: RepeaterItem[] = [];
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

  validationMessage: string = null;

  @Input() isInline: boolean = false;
  @Input() selectedRolloutData: IRolloutRollout;
  @Input() existingRollouts: RolloutRollout[] = [];
  @Input() rolloutImages: IObjstoreObjectList;

  @Output() formClose: EventEmitter<any> = new EventEmitter();

  rolloutNowcheck: boolean = false;
  rolloutDurationcheck: boolean = false;

  versiondescription: string = '';

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    matIcon: 'update'
  };

  naplesEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;
  naples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  naplesIdMap: { [id: string]: ClusterDistributedServiceCard } = {};
  naplesRuleMap: { [rule: string]: Set<string> } = {};
  duplicateMatchMap: { [nicid: string]: number[] } = {};

  repeaterAnimationEnabled: boolean = true;

  constructor(private rolloutService: RolloutService,
    private objstoreService: ObjstoreService,
    protected _searchService: SearchService,
    protected controllerService: ControllerService,
    protected clusterService: ClusterService,
    protected cd: ChangeDetectorRef,
  ) {
    super(controllerService);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  onOrderChange() {
    this.buildDuplicationMap();
    // We disable animation while the re-order animation happens
    // to prevent app-repeater from stuttering
    this.repeaterAnimationEnabled = false;
    setTimeout(() => {
      this.repeaterAnimationEnabled = true;
    }, 500);
  }


  ngOnInit() {
    this.getNaplesLabelsKeyValues();
    // prepare for label-selector repeater
    if (!this.newRollout) {
      // below ngChange() should initialize rolloutData.
      this.initRolloutData();
    }
    this.addOrder();
    this.minDate = new Date();
  }

  // When we get a response containing all DistributedServiceCards objects, we build labelData to be used for the label selector.
  // We also generate a Map of each label-rule to all the matchedDistributedServiceCards.
  handleReadDistributedServiceCardsResponse(response) {
    this.orderConstraintslabelData = [];
    this.naplesEventUtility.processEvents(response);
    this.generateRuleAndIdMap();
    const labels = Utility.getLabels(this.naples as any[]);
    for (const key of Object.keys(labels)) {
      const valList = [];
      for (const val of Object.keys(labels[key])) {
        valList.push({ value: val, label: val });
      }
      const label = {
        key: { value: key, label: key },
        values: valList,
        operators: SearchUtil.stringOperators,
        fieldType: ValueType.singleSelect,
        valueType: ValueType.multiSelect
      };
      this.orderConstraintslabelData.push(label);
    }
  }

  // We also create naplesIdMap which can be used to show matched naples details in the future.
  generateRuleAndIdMap() {
    this.naples.forEach(element => {
      if (element.meta.labels) {
        for (const key of Object.keys(element.meta.labels)) {
          const keyValText = key + '=' + element.meta.labels[key];
          if (!(keyValText in this.naplesRuleMap)) {
            this.naplesRuleMap[keyValText] = new Set<string>();
          }
          this.naplesRuleMap[keyValText].add(element.meta.name);
          this.naplesIdMap[element.meta.name] = element;
        }
      }
    });
  }

  getNaplesLabelsKeyValues() {
    this.naplesEventUtility = new HttpEventUtility<ClusterDistributedServiceCard>(ClusterDistributedServiceCard);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterDistributedServiceCard>;
    const subscription = this.clusterService.WatchDistributedServiceCard().subscribe(
      response => {
        this.handleReadDistributedServiceCardsResponse(response);
      },
      this.controllerService.webSocketErrorHandler('Failed to get labels')
    );
    this.subscriptions.push(subscription);
  }

  initRolloutData() {
    if (!this.isInline) {  // create mode
      this.newRollout = new RolloutRollout();
      this.newRollout.kind = 'rollout';
      const startTime = this.computeDefaultStartTime();
      this.newRollout.spec['scheduled-start-time'] = new Date(startTime);
      const endTime = startTime + 1800 * 1000; // set default 30 minutes rollout duration.
      this.newRollout.spec['scheduled-end-time'] = null; // new Date(endTime);
      this.newRollout.$formGroup.get(['spec', 'suspend']).disable();
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).enable();
      this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).disable();
      this.newRollout.$formGroup.get(['spec', 'retry']).setValue(false);
      this.newRollout.$formGroup.get(['spec', 'retry']).disable();
      this.newRollout.setFormGroupValuesToBeModelValues();

      this.setValidators(this.newRollout);
    } else {   // edit mode
      const myrollout: IRolloutRollout = Utility.getLodash().cloneDeep(this.selectedRolloutData);
      myrollout.spec['scheduled-start-time'] = new Date(myrollout.spec['scheduled-start-time']);
      this.newRollout = new RolloutRollout(myrollout);
      this.newRollout.$formGroup.get(['meta', 'name']).disable();
      this.newRollout.$formGroup.get(['spec', 'version']).disable(); // disable version until version options are available.
      this.selectedRolloutNicNodeTypes = RolloutUtil.getRolloutNaplesVeniceType(this.newRollout);
      this.rolloutNowcheck = false;
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).enable();
    }
    // set validators. TODO: 2019-05-15 wait for server set validator in rollout.proto
    if (this.newRollout.$formGroup.validator) {
      this.newRollout.$formGroup.get(['meta', 'name']).setValidators([required, this.isRolloutNameValid(this.existingRollouts), this.newRollout.$formGroup.validator]);
    } else {
      this.newRollout.$formGroup.get(['meta', 'name']).setValidators([required, this.isRolloutNameValid(this.existingRollouts)]);
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).setValidators([required, this.isRolloutScheduleTimeValid()]);
      this.newRollout.$formGroup.get(['spec', 'version']).setValidators([required]);
    }
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      this.setToolbarButtons();
    }
  }

  setValidators(newRollout: RolloutRollout) {
    newRollout.$formGroup.get(['meta', 'name']).setValidators([
      this.newRollout.$formGroup.get(['meta', 'name']).validator,
      this.isNewRolloutNameValid(this.existingObjects)]);
  }

  isNewRolloutNameValid(existingObjects: IRolloutRollout[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newRollout-name');
  }

  computeDefaultStartTime(): number {
    const startTime = new Date().getTime() + 300 * 1000; // VS-331 set default time. Now + 5 min
    return startTime;
  }

  computeDefaultEndTime() {
    if (!this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value) {
      // when end-time is not set, we compute the end-time
      const startTime = (this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value) ? this.computeDefaultStartTime() : this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value;
      const endTime = this.computeDefaultEndTimeByStartTime(startTime);
      this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).setValue(new Date(endTime));
    }
  }

  computeDefaultEndTimeByStartTime(startTime: number): number {
    return startTime + 1800 * 1000; // set default 30 minutes rollout duration.
  }


  ngAfterViewInit() {
  }

  private setToolbarButtons() {
    this.oldToolbarData = this.controllerService.getToolbarData();
    const newToolbarData = this.controllerService.getToolbarData();
    const newbuttons = [
      {
        cssClass: 'global-button-primary newrollout-toolbar-button',
        text: 'SAVE ROLLOUT',
        genTooltip: () => this.getTooltip(),
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
    newToolbarData.buttons = newbuttons;
    this.controllerService.setToolbarData(newToolbarData);
  }

  getTooltip(): string {
    return Utility.isEmpty(this.validationMessage) ? 'Ready to save rollout' : this.validationMessage;
  }

  isRolloutScheduleTimeValid(): ValidatorFn {
    const greateThan = 1;
    const now = new Date();
    return Utility.dateValidator(now, greateThan, 'schedule-time', 'Can not schedule rollout to the past');
  }

  isRolloutNameValid(existingRollouts: RolloutRollout[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingRollouts, 'rollout-name');
  }

  countMatchedNICs() {
    let count = 0;
    for (const order of this.orders) {
      count += order.matchedDistributedServiceCards.length;
    }
    return count;
  }

  getOrderConstraints() {
    return this.orders.map(x => x.data);
  }

  /**
   * Input validation logic
   * 1. meta.name is required
   * 2. spec.version is required
   * 3. if DSC retry is checked, both end-time and max-failure inputs are required
   * 4. check DSC constraints
   * 5. start-time < end-time and the duration should reasonable.
   *
   */
  isAllInputsValidated(): boolean {
    this.validationMessage = null;
    if (!this.newRollout || !this.newRollout.$formGroup) {
      return false;
    }
    if (Utility.isEmpty(this.newRollout.$formGroup.get(['meta', 'name']).value)) {
      this.validationMessage = 'Rollout name is required.';
      return false;
    }
    if (this.newRollout.$formGroup['dsc-must-match-constraint'] && this.countMatchedNICs() !== 0) {
      this.validationMessage = 'No matching DSCs were found for the given constraints.';
      return false;
    }
    /* 2019-12-09, product time want to have -  “ I do not think End Time should be mandatory, if a user select "Continuous Retry".
     comment out the following block for now.
    if (!this.isRetryEndTimeValueCorrect()) {
      this.validationMessage = 'An end time is required when "retry failed DSCs" is true.';
      return false;
    } */
    if (this.newRollout.$formGroup.get(['spec', 'dsc-must-match-constraint']).value) {

      const orders = this.getOrderConstraints();
      for (let ix = 0; ix < orders.length; ix++) {
        const repeaterSearchExpression: SearchExpression[] = this.convertFormArrayToSearchExpression(orders[ix].value);
        if (repeaterSearchExpression.length === 0) {
          this.validationMessage = 'No DSC search criteria is specified.';
          return false;
        } else {
          for (let i = 0; i < repeaterSearchExpression.length; i++) {
            if (Utility.isEmpty(repeaterSearchExpression[i].key) || Utility.isEmpty(repeaterSearchExpression[i].values)) {
              this.validationMessage = 'DSC constraint has input errors';
              return false;
            }
          }
        }
      }
    }
    if (!this.isStartTimeEndTimeValuesCorrect()) {
      this.validationMessage = 'Start time should be less than end time. Check inputs ';
      return false;
    }
    const diff = this.computeDuration();
    if (diff !== null && diff < 30) {
      this.validationMessage = 'Rollout duration should be at least 30 minutes.';
      return false;
    }
    if (Utility.isEmpty(this.newRollout.$formGroup.get(['spec', 'version']).value)) {
      this.validationMessage = 'Must select rollout version';
      return false;
    }
    const hasFormGroupError = Utility.getAllFormgroupErrors(this.newRollout.$formGroup);
    return hasFormGroupError === null;
  }

  isRetryEndTimeValueCorrect(): boolean {
    if (this.selectedRolloutNicNodeTypes !== RolloutUtil.ROLLOUTTYPE_VENICE_ONLY) {
      // if rollout type is  both-Venice+DSC or DSC only and spec.retry is true, user must specify rollout.spec.end-time
      if (this.newRollout.$formGroup.get(['spec', 'retry']).value) {
        if (!this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * When user does not specify rollout duration, end-time input is not required.
   * Otherwise, we have to make sure end-time is higher than start-time.
   */
  isStartTimeEndTimeValuesCorrect(): boolean {
    if (!this.rolloutDurationcheck) {
      if (this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value &&
        this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value >= this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value) {
        return false;
      }
    } else {
      if (this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value &&
        this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value &&
        this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value >= this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value) {
        return false;
      }
    }
    return true;
  }

  computeDuration(): number {
    if (this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value &&
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value) {
      const startTime = this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).value;
      const entTime = this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).value;
      const moment = Utility.getMomentJS();
      const diff = moment(entTime).diff(moment(startTime), 'minutes');
      return diff;
    } else {
      return null;
    }
  }

  // Example:
  // orderValue = {
  // requirements: [{
  //   keyFormControl: "key",
  //   operatorFormControl: "in",
  //   valueFormControl: ["1"]}]
  // }
  convertFormArrayToSearchExpression(orderValue: any, addMetatag: boolean = false): SearchExpression[] {
    return SearchUtil.convertFormArrayToSearchExpression(orderValue.requirements, addMetatag);
  }

  onRolloutDurationCheckChange(checked) {
    this.rolloutDurationcheck = checked;
    if (checked) {
      this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).enable();
      this.computeDefaultEndTime();
    } else {
      this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).disable();
      this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).setValue(null);
    }
  }

  onRolloutNowChange(checked) {
    this.rolloutNowcheck = checked;
    if (checked) {
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).disable();
    } else {
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).enable();
    }
  }

  /**
   * This API serves HTML template
   * @param checked
   *
   * If user check RETRY checkbox, we want user to enter end-time
   */
  onRolloutDSCRetryChange(checked) {
    if (checked) {
      // VS-974 comment this line for now -- this.newRollout.$formGroup.get(['spec', 'scheduled-end-time']).enable();

      // Per VS-898, we want to let user upgrade DSC with RETRY option without setting end time.

      /* comment out this block for now.
      this.rolloutDurationcheck = true;
      this.computeDefaultEndTime();
      if (this.checkboxSetDuration) {
        this.checkboxSetDuration.checked = true;  // set the Duration-check to UI checked state.
        this.checkboxSetDuration.disabled = true;
      }
      */
    } else {
      this.newRollout.$formGroup.get(['spec', 'scheduled-start-time']).enable();
    }
  }

  /**
   * This API serves HTML template
   * Data flow logic:
   * In UI, display “Max allowed DSCs Failures” before “retry” box.
   * Retry can only be enabled when “Max allowed DSCs Failures” is non-zero, if “Max Allowed DSCs Failures” is 0, grey out the “retry” box.
   * If “retry” is enabled and user then changes the max allowed DSCs failures to 0, grey out “retry” box again
   * @param $event
   */
  onMaxAllowDSCFailureChange($event) {
    if (!!(this.newRollout.$formGroup.get(['spec', 'max-nic-failures-before-abort']).value)) {  // max-nic-failures-before-abort > 0
      this.newRollout.$formGroup.get(['spec', 'retry']).enable();
    } else {
      this.newRollout.$formGroup.get(['spec', 'retry']).setValue(false);
      this.newRollout.$formGroup.get(['spec', 'retry']).disable();
    }
  }

  /**
   * This API serves html template
   * When DSC.retry is selected, disable rollout duration checkbox.
   * 2019-11-21, this function is not registered in html. QA wants to test  DSD.retry without end-time.
   */
  isToDisableDurationCheckbox(): boolean {
    // add back [disabled]="isToDisableDurationCheckbox()" to  html #checkboxSetDuration, once QA wants to force DSD.retry with end-time.
    if (this.selectedRolloutNicNodeTypes !== RolloutUtil.ROLLOUTTYPE_VENICE_ONLY) {
      if (!!this.newRollout.$formGroup.get(['spec', 'retry']).value) {
        return true;
      }
    }
    return false;
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
    if (this.newRollout.$formGroup.value.spec['dsc-must-match-constraint']) {
      let duplicate = false;
      for (const order of this.orders) {
        if (order.duplicatesWith.length > 0) {
          duplicate = true;
          break;
        }
      }
      if (duplicate) {
        this._controllerService.invokeConfirm({
          header: 'Multiple orders matching some DSCs.',
          message: 'The earliest order in the sequence will take preference for each of these DSCs. Do you want to make some changes?',
          acceptLabel: 'Yes',
          rejectLabel: 'No',
          reject: () => {
            this._saveRollout();
          }
        });
      } else {
        this._saveRollout();
      }
    } else {
      this._saveRollout();
    }
  }

  updateRollout() {
    this._saveRollout();
  }

  /**
   * Build rollout JSON.
   * according to RolloutUtil.getRolloutNaplesVeniceType() rules
   * sample working rollout ( upgrade some Naples)
   * {"kind":"rollout","api-version":null,"meta":{"name":"jeff-vs529-3"},"spec":{"version":"0.11.0-55","scheduled-start-time":"2019-06-25T22:56:21.145Z","strategy":"LINEAR","max-parallel":2,"max-nic-failures-before-abort":1,"order-constraints":[{"requirements":[{"key":"number","operator":"equals","values":["1"]}]}],"dscs-only":false,"dsc-must-match-constraint":true,"upgrade-type":"Disruptive"}}
   */
  buildRollout(): IRolloutRollout {
    const rollout: IRolloutRollout = this.newRollout.getFormGroupValues();
    if (this.rolloutNowcheck) {
      const today = new Date().getTime() + 30 * 1000; // If user wants to run rollout right the way, we give 30 seconds.
      rollout.spec['scheduled-start-time'] = new Date(today);
    }
    rollout.meta.name = (rollout.meta.name) ? rollout.meta.name : this.newRollout.meta.name;
    if (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_VENICE_ONLY) {
      rollout.spec['max-nic-failures-before-abort'] = null;
      rollout.spec['order-constraints'] = [];
      rollout.spec['dsc-must-match-constraint'] = true;
      rollout.spec['dscs-only'] = false;
      rollout.spec['max-parallel'] = null; // 	VS-769. When rollout is Venice-only, there max-parallel does not apply
      rollout.spec.retry = false;
    } else if (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_NAPLES_ONLY) {
      rollout.spec['dscs-only'] = true;
      this.setSpecOrderConstrains(rollout);
    } else if (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE) {
      rollout.spec['dscs-only'] = false;
      this.setSpecOrderConstrains(rollout);
    }
    return rollout;
  }

  /**
   *
   * @param rollout
   * order-constraints”: [
   *  {
   *    “requirements”: [
   *     {
   *      “key”: “number”,
   *       “operator”: “equals”,
   *       “values”: [
   *          “1"
   *       ]
   *     }
   *   ]
   * }
   */
  private setSpecOrderConstrains(rollout: IRolloutRollout) {
    if (rollout.spec['dsc-must-match-constraint']) {
      const orderConstraints = [];
      const orders = this.getOrderConstraints();
      for (let ix = 0; ix < orders.length; ix++) {
        const labelsSelectorCriteria = this.convertFormArrayToSearchExpression(orders[ix].value); // Some Naples will be updated.
        const requirements = labelsSelectorCriteria;
        const obj = { 'requirements': requirements };
        orderConstraints.push(obj);
      }
      rollout.spec['order-constraints'] = orderConstraints;
    } else {
      rollout.spec['order-constraints'] = []; // All Naples will be updated.
    }
  }

  _saveRollout() {
    const rollout: IRolloutRollout = this.buildRollout();
    let handler: Observable<{ body: IRolloutRollout | IApiStatus | Error, statusCode: number }>;

    if (this.isInline) {
      // Using this.newRollout to get name, as the name is gone when we call getFormGroupValues
      // This is beacuse we disabled it in the form group to stop the user from editing it.
      // When you disable an angular control, in doesn't show up when you get the value of the group

      handler = this.rolloutService.UpdateRollout(rollout);
    } else {
      handler = this.rolloutService.CreateRollout(rollout);
    }
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
    return this.newRollout.$formGroup.get(['spec', 'dsc-must-match-constraint']).value;
  }

  /**
   * Customized placeholder for key
   * @param repeater
   * @param keyFormName
   */
  buildKeyPlaceholder(repeater: any, keyFormName: string): string {
    return 'key';
  }

  /**
   * Customized placeholder value
   * @param repeater
   * @param keyFormName
   */
  buildValuePlaceholder(repeater: any, keyFormName: string): string {
    return 'value';
  }

  // adds new HTML element with labelselectors
  // automatically collapses all open orders
  addOrder() {
    const newOrder = new RolloutOrder();
    newOrder.id = Utility.s4();
    newOrder.data = new LabelsSelector().$formGroup;
    this.orders.push(newOrder);
    this.makeOrderEditable(this.orders.length - 1, null);
    this.orders[this.orders.length - 1].incomplete = true;
  }

  // removes order
  deleteOrder(index) {
    this.orders.splice(index, 1);
  }

  // toggle order state between saved and editable
  // Make a saved order editable will make all other orders closed.
  toggleOrderState(index) {
    if (this.orders[index].inEdit && this.orders.length > 1) {
      this.makeOrderSaved(index);
    } else {
      this.makeOrderEditable(index, null);
    }
  }

  // Only one order can be edited at one time.
  // If we create a new order or edit an existing one, previously open order is automatically saved.
  makeOrderEditable(index, event) {
    for (let i = 0; i < this.orders.length; i++) {
      if (i !== index && this.orders[i].inEdit) {
        this.makeOrderSaved(i);
      } else if (i === index) {
        this.orders[index].inEdit = true;
      }
    }
    if (event) {
      event.stopPropagation();
    }
  }

  // "save" simply hides the label selectors
  // save updates the order summary string and the matchedNIC count.
  makeOrderSaved(index) {
    this.orders[index].inEdit = false;
    this.matchNICs(index);
    this.setOrderSummary(index);
    this.buildDuplicationMap();
  }

  // saved orders show a summary of the label selectors contained inside.
  setOrderSummary(index) {
    const data = this.orders[index].data.value.requirements;
    const reqs = Utility.formatRepeaterData(data);
    const stringForm = Utility.stringifyRepeaterData(reqs).join('    ');
    this.orders[index].orderSummary = stringForm;
  }

  moveOrder(index, direction) {
    const newIndex = index + direction;

    if (newIndex < 0 || newIndex >= this.orders.length) {
      return;
    }

    const tempC = this.orders[index];
    this.orders[index] = this.orders[newIndex];
    this.orders[newIndex] = tempC;
  }

  repeaterValueChange(index) {
    setTimeout(() => {
      this.matchNICs(index);
      this.setOrderSummary(index);
      this.buildDuplicationMap();
      const repeaterSearchExpression: SearchExpression[] = this.convertFormArrayToSearchExpression(this.orders[index].data.value);
      if (repeaterSearchExpression.length > 0) {
        this.orders[index].incomplete = false;
        for (let i = 0; i < repeaterSearchExpression.length; i++) {
          if (Utility.isEmpty(repeaterSearchExpression[i].key) || Utility.isEmpty(repeaterSearchExpression[i].values)) {
            this.orders[index].incomplete = true;
          }
        }
      } else {
        this.orders[index].incomplete = true;
      }
    }, 0);
  }

  matchNICs(index) {
    const repeaterValues = this.orders[index].data.value.requirements;
    const matchRules = [];
    for (let ix = 0; ix < repeaterValues.length; ix++) {
      matchRules.push([repeaterValues[ix].keyFormControl, repeaterValues[ix].operatorFormControl, repeaterValues[ix].valueFormControl]);
    }
    this.updateMatches(index, matchRules);
  }

  makeRuleMapKey(labelKey, labelValue) {
    return labelKey + '=' + labelValue;
  }

  // Compares the labelselector rules with the naplesRuleMap to get all matched NICs
  getRuleResults(matchRule) {
    const ok: boolean = true;
    let naplesNICSet: string[] = [];
    if (typeof matchRule[2] === 'object' && !!matchRule[2]) {
      for (const i of matchRule[2]) {
        const rule = this.makeRuleMapKey(matchRule[0], i);
        if (rule in this.naplesRuleMap) {
          naplesNICSet = naplesNICSet.concat(Array.from(this.naplesRuleMap[rule]));
        }
      }
    } else {
      const rule = this.makeRuleMapKey(matchRule[0], matchRule[2]);
      if (rule in this.naplesRuleMap) {
        naplesNICSet = Array.from(this.naplesRuleMap[rule]);
      }
    }

    if (naplesNICSet.length > 0) {
      return naplesNICSet;
    } else {
      return null;
    }
  }

  getAllNICs() {
    const naplesNICSet: Set<string> = new Set<string>();
    for (const item of this.naples) {
      naplesNICSet.add(item.meta.name);
    }
    return Array.from(naplesNICSet);
  }

  updateMatches(index, matchRules) {
    let naplesNICSet: string[] = null;
    for (const matchRule of matchRules) {
      const ret = this.getRuleResults(matchRule);
      if (ret === null) {
        break; // since all rules are 'AND'ed together. If one matches nothing, we dont have to check for the rest.
      }
      if (matchRule[1] === 'in') {
        if (naplesNICSet === null) {
          naplesNICSet = Array.from(ret);
        }
        naplesNICSet = naplesNICSet.filter(x => Array.from(ret).includes(x));
      } else {
        if (naplesNICSet === null) {
          naplesNICSet = this.getAllNICs();
        }
        naplesNICSet = naplesNICSet.filter(x => !Array.from(ret).includes(x));
      }
    }

    if (naplesNICSet === null) { this.orders[index].matchedDistributedServiceCards = []; } else { this.orders[index].matchedDistributedServiceCards = naplesNICSet; }
  }

  // If there are no labels added (for any of the NICs), the upgrade using labels toggle button is hidden.
  enableUpgradeByLabel() {
    if (Object.keys(this.naplesRuleMap).length > 0) {
      return true;
    } else {
      return false;
    }
  }

  duplicateMatchTooltip(index) {
    let msg = 'Has common matches with Order';
    msg += this.orders[index].duplicatesWith.length > 1 ? 's ' : ' ';
    const arr = this.orders[index].duplicatesWith.map((value) => {
      return value + 1;
    });
    msg += arr.join(', ');
    return msg;
  }

  buildDuplicationMap() {
    this.duplicateMatchMap = {};
    for (let ix = 0; ix < this.orders.length; ix++) {
      const order = this.orders[ix];
      this.orders[ix].duplicatesWith = [];
      for (const match of order.matchedDistributedServiceCards) {
        if (!(match in this.duplicateMatchMap)) {
          this.duplicateMatchMap[match] = [ix];
        } else {
          this.orders[ix].duplicatesWith = this.orders[ix].duplicatesWith.concat([...this.duplicateMatchMap[match]]);
          this.duplicateMatchMap[match].push(ix);
        }
      }
      // sort and get unique
      this.orders[ix].duplicatesWith = this.orders[ix].duplicatesWith.sort().filter(function (el, i, a) { return i === a.indexOf(el); });
    }
  }
}
