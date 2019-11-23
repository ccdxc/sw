import { Component, OnInit, ViewEncapsulation, AfterViewInit, Input } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import {
  IMonitoringMirrorSession, MonitoringMirrorSession, ILabelsRequirement,
  MonitoringMirrorSessionSpec, MonitoringMirrorCollector, MonitoringMatchRule
} from '@sdk/v1/models/generated/monitoring';
import { SecurityApp } from '@sdk/v1/models/generated/security';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import {
  AbstractControl, FormArray, FormControl, ValidatorFn, Validators, ValidationErrors
} from '@angular/forms';
import { IPUtility } from '@app/common/IPUtility';
import { Utility } from '@app/common/Utility';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { RepeaterData, ValueType } from 'web-app-framework';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { OrderedItem } from '@app/components/shared/orderedlist/orderedlist.component';
import { SelectItem } from 'primeng/api';
import { HttpEventUtility } from '@app/common/HttpEventUtility';

const PACKET_FILTERS_ERRORMSG: string =
    'At least one match rule must be specified if packet filter is set to All Packets.';

@Component({
  selector: 'app-newmirrorsession',
  templateUrl: './newmirrorsession.component.html',
  styleUrls: ['./newmirrorsession.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
})
export class NewmirrorsessionComponent extends CreationForm<IMonitoringMirrorSession, MonitoringMirrorSession> implements OnInit, AfterViewInit {

  @Input() existingObjects: MonitoringMirrorSession[] = [];

  createButtonTooltip: string = '';
  minDate: Date = new Date();

  labelData: RepeaterData[] = [];
  labelOutput: any;
  labelFormArray = new FormArray([]);

  packetFilterOptions = Utility.convertEnumToSelectItem(MonitoringMirrorSessionSpec.propInfo['packet-filters'].enum);
  collectorTypeOptions = Utility.convertEnumToSelectItem(MonitoringMirrorCollector.propInfo['type'].enum);

  repeaterAnimationEnabled = true;
  rules: OrderedItem<any>[] = [];

  PROTO_PORTS_OPTION: string = 'proto-ports';
  APPS_OPTION: string = 'applications';

  protoAppOptions = [
    {label: 'PROTO-PORTS', value: this.PROTO_PORTS_OPTION},
    {label: 'APPS', value: this.APPS_OPTION},
  ];

  securityAppsEventUtility: HttpEventUtility<SecurityApp>;
  securityApps: ReadonlyArray<SecurityApp> = [];
  securityAppOptions: SelectItem[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    protected securityService: SecurityService,
    protected uiconfigsService: UIConfigsService,
  ) {
    super(_controllerService, uiconfigsService, MonitoringMirrorSession);
  }

  getSecurityApps() {
    this.securityAppsEventUtility = new HttpEventUtility<SecurityApp>(SecurityApp, false, null, true); // https://pensando.atlassian.net/browse/VS-93 we want to trim the object
    this.securityApps = this.securityAppsEventUtility.array as ReadonlyArray<SecurityApp>;
    const subscription = this.securityService.WatchApp().subscribe(
      response => {
        this.securityAppsEventUtility.processEvents(response);
        this.securityAppOptions = this.securityApps.map(app => {
          return {
            label: app.meta.name,
            value: app.meta.uuid,
          };
        });
      },
      this._controllerService.webSocketErrorHandler('Failed to get Apps')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit(): void {
    this.getSecurityApps();
    this.labelData = [
      {
        key: { label: 'text', value: '' },
        operators: [{ label: 'equals', value: 'in' }],
        fieldType: ValueType.inputField,
        valueType: ValueType.inputField
      }
    ];

    if (this.isInline) {
      // conver date from staring to Date Object
      const dateValue = this.newObject.$formGroup.get(['spec', 'start-condition', 'schedule-time']).value;
      if (dateValue) {
        this.newObject.$formGroup.get(['spec', 'start-condition', 'schedule-time']).setValue(
          new Date(dateValue));
      }

      // process interface selectors
      if (this.objectData.spec['interface-selector']) {
        const searchExpressons: ILabelsRequirement[] =
          this.objectData.spec['interface-selector'].requirements;
        const list = [];
        searchExpressons.forEach((item) => {
          const op = item.operator;
          const formControl = new FormControl({
            keyFormControl: '',
            operatorFormControl: op,
            valueFormControl: (item.values) ? item.values : [],
            keytextFormName: item.key
          });
          list.push(formControl);
        });
        this.labelFormArray = new FormArray(list);
      }

      // process match rules
      this.rules = [];
      if (this.newObject.spec['match-rules'].length > 0) {
        this.newObject.spec['match-rules'].forEach(rule => {
          rule.$formGroup.valueChanges.subscribe(() => {
            this.newObject.$formGroup.get(['spec', 'packet-filters']).updateValueAndValidity();
          });
          let selectedProtoAppOption = this.PROTO_PORTS_OPTION;
          if (rule['app-protocol-selectors'].applications.length > 0) {
            selectedProtoAppOption = this.APPS_OPTION;
          }
          this.rules.push({
            id: Utility.s4(),
            data: { rule, selectedProtoAppOption },
            inEdit: false
          });
        });
      }
      // disable name field
      this.newObject.$formGroup.get(['meta', 'name']).disable();
    }

    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isMirrorsessionNameValid(this.existingObjects)]);

    this.newObject.$formGroup.get(['spec', 'packet-filters']).setValidators([
      Validators.required,
      this.packetFiltersValidator()]);

    // Add one collectors if it doesn't already have one
    const collectors = this.newObject.$formGroup.get(['spec', 'collectors']) as FormArray;
    if (collectors.length === 0) {
      this.addCollector();
    }

    // Add one matchrule if it doesn't already have one
    if (this.rules.length === 0) {
      this.addRule();
    }
  }

  getPacketFiltersTooltip(): string {
    const packetFiltersField: FormControl =
      this.newObject.$formGroup.get(['spec', 'packet-filters']) as FormControl;
    if (Utility.isEmpty(packetFiltersField.value)) {
      return '';
    }
    if (!packetFiltersField.valid) {
      return PACKET_FILTERS_ERRORMSG;
    }
    return '';
  }

  isFieldEmpty(field: AbstractControl): boolean {
    return Utility.isEmpty(field.value);
  }

  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['spec', 'packet-filters']).value)) {
      this.createButtonTooltip = 'Error: Packet Filters field is empty.';
      return false;
    }

    if (!this.newObject.$formGroup.get(['spec', 'packet-filters']).valid) {
      this.createButtonTooltip = PACKET_FILTERS_ERRORMSG;
      return false;
    }

    const collectors = this.controlAsFormArray(
      this.newObject.$formGroup.get(['spec', 'collectors'])).controls;
    for (let i = 0; i < collectors.length; i++) {
      const collector = collectors[i];
      if (Utility.isEmpty(collector.get(['export-config', 'destination']).value)) {
        this.createButtonTooltip = 'Error: Collector Destination is required.';
        return false;
      }
    }

    for (let i = 0; i < this.rules.length; i++) {
      const rule: MonitoringMatchRule = this.rules[i].data.rule;
      if (!(rule.$formGroup.get(['source', 'ip-addresses']).valid)) {
        this.createButtonTooltip =
          'Error: Rule ' + (i + 1) + ' source IP adresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['source', 'mac-addresses']).valid)) {
        this.createButtonTooltip =
          'Error: Rule ' + (i + 1) + ' source MAC adresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['destination', 'ip-addresses']).valid)) {
        this.createButtonTooltip =
          'Error: Rule ' + (i + 1) + ' destination IP adresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['destination', 'mac-addresses']).valid)) {
        this.createButtonTooltip =
          'Error: Rule ' + (i + 1) + ' destination MAC adresses are invalid.';
        return false;
      }
      if (!(rule.$formGroup.get(['app-protocol-selectors', 'proto-ports']).valid)) {
        this.createButtonTooltip =
          'Error: Rule ' + (i + 1) + ' protocol/ports are invalid.';
        return false;
      }
    }

    if (!this.newObject.$formGroup.valid) {
      this.createButtonTooltip = 'Error: Form is invalid.';
      return false;
    }

    this.createButtonTooltip = '';
    return true;
  }

  getTooltip(): string {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return 'Error: Name field is empty.';
    }
    if (!this.newObject.$formGroup.get(['meta', 'name']).valid)  {
      return 'Error: Name field is invalid.';
    }
    return this.createButtonTooltip;
  }

  getObjectValues() {
    const currValue: IMonitoringMirrorSession = this.newObject.getFormGroupValues();
    currValue.spec['match-rules'] = this.rules.map(r => {
      return r.data.rule.getFormGroupValues();
    });
    const repeaterSearchExpression = SearchUtil.convertFormArrayToSearchExpression(this.labelOutput);
    currValue.spec['interface-selector'].requirements = repeaterSearchExpression as ILabelsRequirement[];
    return currValue;
  }

  // to change the default placeholder string
  buildKeyPlaceholder(): string {
    return 'Enter Label Key';
  }

  // to change the default placeholder string
  buildValuePlaceholder(): string {
    return 'Enter Label Value';
  }

  protected buildLabelFormControlList(searchExpressons: ILabelsRequirement[]): FormControl[] {
    const list = [];
    searchExpressons.forEach((item) => {
      const op = item.operator;
      const formControl = new FormControl({
        keyFormControl: '',
        operatorFormControl: op,
        valueFormControl: (item.values) ? item.values : [],
        keytextFormName: item.key
      });
      list.push(formControl);
    });
    return list;
  }

  isMirrorsessionNameValid(existingTechSupportRequest: MonitoringMirrorSession[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingTechSupportRequest, 'Mirror-session-name');
  }

  packetFiltersValidator() {
    return (control: AbstractControl): ValidationErrors | null => {
      const arr: string[] = control.value;
      if (arr.indexOf('all-packets') > -1 && this.areAllRulesEmpty()) {
        return {
          packetFilters: {
            required: true,
            message: PACKET_FILTERS_ERRORMSG
          }
        };
      }
      return null;
    };
  }

  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary newmirrorsession-toolbar-button newmirrorsession-toolbar-SAVE',
        text: 'CREATE MIRROR SESSION',
        matTooltipClass: 'validation_error_tooltip',
        genTooltip: () => this.getTooltip(),
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass()
      },
      {
        cssClass: 'global-button-neutral newmirrorsession-toolbar-button newmirrorsession-toolbar-CANCEL',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  /**
   * This API serves html template. When repeater's value is changed, we update labelOutput value.
   */
  handleLabelRepeaterData(values: any) {
    this.labelOutput = values;
  }

  addCollector() {
    const collectors = this.newObject.$formGroup.get(['spec', 'collectors']) as FormArray;
    const newCollector = new MonitoringMirrorCollector().$formGroup;
    collectors.insert(collectors.length, newCollector);
  }

  removeCollector(index: number) {
    const collectors = this.newObject.$formGroup.get(['spec', 'collectors']) as FormArray;
    if (collectors.length > 1) {
      collectors.removeAt(index);
    }
  }

  onRadioButtonChange(event, data) {
    if (event.value === 'applications') {
      data.rule.$formGroup.get(['app-protocol-selectors', 'proto-ports']).setValue([]);
    } else {
      data.rule.$formGroup.get(['app-protocol-selectors', 'applications']).setValue([]);
    }
    data.selectedProtoAppOption = event.value;
  }

  addRule() {
    const rule = new MonitoringMatchRule();
    rule.$formGroup.valueChanges.subscribe(() => {
      this.newObject.$formGroup.get(['spec', 'packet-filters']).updateValueAndValidity();
    });
    this.rules.push({
      id: Utility.s4(),
      data: { rule: rule, selectedProtoAppOption: this.PROTO_PORTS_OPTION },
      inEdit: false
    });
    this.editRule(this.rules.length - 1);
  }

  editRule(index) {
    // Collapse any other open rules, and make index rule open
    this.rules.forEach( (r, i) => {
      if (i === index) {
        r.inEdit = true;
        this.addValidatorsToRule(r.data.rule);
      } else {
        r.inEdit = false;
      }
    });
  }

  deleteRule(index) {
    if (this.rules.length > 1) {
      this.rules.splice(index, 1);
    }
  }

  orderedListClick(index) {
    // Only toggle if the rule we are clicking on isn't in edit mode
    if (!this.rules[index].inEdit) {
      this.editRule(index);
    }
  }

  areAllRulesEmpty() {
    for (let i = 0; i < this.rules.length; i++) {
      if (!this.isRuleEmpty(this.rules[i].data.rule)) {
        return false;
      }
    }
    return true;
  }

  isRuleEmpty(rule: MonitoringMatchRule) {
    const fields = [
      rule.$formGroup.get(['source', 'ip-addresses']),
      rule.$formGroup.get(['source', 'mac-addresses']),
      rule.$formGroup.get(['destination', 'ip-addresses']),
      rule.$formGroup.get(['destination', 'mac-addresses']),
      rule.$formGroup.get(['app-protocol-selectors', 'proto-ports']),
      rule.$formGroup.get(['app-protocol-selectors', 'applications'])
    ];
    for (let i = 0; i < fields.length; i++) {
      if (fields[i].valid && !Utility.isEmpty(fields[i].value)) {
        return false;
      }
    }
    return true;
  }

  addValidatorsToRule(rule: MonitoringMatchRule) {
    let field = rule.$formGroup.get(['source', 'ip-addresses']);
    if (!field.validator) {
      field.setValidators([this.isIpAddressesValid()]);
    }
    field = rule.$formGroup.get(['source', 'mac-addresses']);
    if (!field.validator) {
      field.setValidators([this.isMacAddressesValid()]);
    }
    field = rule.$formGroup.get(['destination', 'ip-addresses']);
    if (!field.validator) {
      field.setValidators([this.isIpAddressesValid()]);
    }
    field = rule.$formGroup.get(['destination', 'mac-addresses']);
    if (!field.validator) {
      field.setValidators([this.isMacAddressesValid()]);
    }
    field = rule.$formGroup.get(['app-protocol-selectors', 'proto-ports']);
    if (!field.validator) {
      field.setValidators([this.isProtocolsValid()]);
    }
  }

  isIpAddressesValid(): ValidatorFn {
    return this.isRuleFieldValid(this.isValidIP);
  }

  isMacAddressesValid(): ValidatorFn {
    return this.isRuleFieldValid(this.isValidMAC);
  }

  isProtocolsValid(): ValidatorFn {
    return this.isRuleFieldValid(this.isValidProto);
  }

  isRuleFieldValid(itemValidator: (val: string) => boolean): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const arr: string[] = control.value;
      if (!arr) {
        return null;
      }
      for (let i = 0; i < arr.length; i++) {
        if (arr[i] && !itemValidator(arr[i])) {
          return {
            field: {
              required: false,
              message: 'Invalid IP addresses'
            }
          };
        }
      }
      return null;
    };
  }

  getAppArray(data: any) {
    if (!this.securityAppOptions || this.securityAppOptions.length === 0) {
      return [];
    }
    const arr = data.rule.$formGroup.get(['app-protocol-selectors', 'applications']).value;
    return arr.map(id => {
      const obj: SelectItem = this.securityAppOptions.find(item => item.value === id);
      return obj.label;
    });
  }

  getDataArray(data: any, type: string, field: string) {
    if (type === 'app-protocol-selectors' && field === 'applications') {
      return this.getAppArray(data);
    }
    return data.rule.$formGroup.get([type, field]).value;
  }

  isValidIP(ip: string) {
    return IPUtility.isValidIPWithOptionalMask(ip);
  }

  isValidMAC(mac: string) {
    return Utility.MACADDRESS_REGEX.test(mac);
  }

  isValidProto(chip: string) {
    const arr: string[] = chip.split('/');
    const prot: string = arr[0];
    if (arr.length !== 2) {
      return true;
    }
    if (prot.toLowerCase() !== 'tcp' &&
        prot.toLowerCase() !== 'udp' &&
        prot.toLowerCase() !== 'dns' &&
        prot.toLowerCase() !== 'ftp' &&
        prot.toLowerCase() !== 'sunrpc' &&
        prot.toLowerCase() !== 'msrpc') {
      return false;
    }
    const number = parseInt(arr[1], 10);
    if (isNaN(number)) {
      return false;
    }
    if (number < 1 || number > 65536) {
      return false;
    }
    return true;
  }

  createObject(newObject: IMonitoringMirrorSession) {
    return this._monitoringService.AddMirrorSession(newObject);
  }

  updateObject(newObject: IMonitoringMirrorSession, oldObject: IMonitoringMirrorSession) {
    return this._monitoringService.UpdateMirrorSession(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: IMonitoringMirrorSession): string {
    return 'Created mirror session ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringMirrorSession): string {
    return 'Updated mirror session ' + object.meta.name;
  }

}
