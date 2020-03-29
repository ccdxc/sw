import { Component, OnInit, ViewEncapsulation, AfterViewInit, Input, ChangeDetectionStrategy } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import {
  IMonitoringMirrorSession, MonitoringMirrorSession, ILabelsRequirement,
  MonitoringMirrorSessionSpec, MonitoringMirrorCollector, MonitoringMatchRule,
  MonitoringMirrorSessionSpec_packet_filters
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
import { minValueValidator } from '@sdk/v1/utils/validators';
import { NetworkNetworkInterface, NetworkNetworkInterfaceSpec_type } from '@sdk/v1/models/generated/network';
import { NetworkService } from '@app/services/generated/network.service';
import {SearchExpression, SearchInputTypeValue, SearchModelField, SearchSpec} from '@app/components/search';
import { LabelsSelector, ILabelsSelector } from '@sdk/v1/models/generated/monitoring/labels-selector.model';
import { MonitoringInterfaceMirror, IMonitoringInterfaceMirror } from '@sdk/v1/models/generated/monitoring/monitoring-interface-mirror.model';


const PACKET_FILTERS_ERRORMSG: string =
  'At least one match rule must be specified';
  // Since Packet Filter UI control is not shown, this part of the string is not to be shown yet"
  // + 'if packet filter is set to All Packets.';

@Component({
  selector: 'app-newmirrorsession',
  templateUrl: './newmirrorsession.component.html',
  styleUrls: ['./newmirrorsession.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
  changeDetection: ChangeDetectionStrategy.OnPush,
})
export class NewmirrorsessionComponent extends CreationForm<IMonitoringMirrorSession, MonitoringMirrorSession> implements OnInit, AfterViewInit {

  @Input() existingObjects: MonitoringMirrorSession[] = [];

  IPS_LABEL: string = 'IP Addresses';
  IPS_ERRORMSG: string = 'Invalid IP addresses';
  IPS_TOOLTIP: string = 'Type in ip address and hit enter or space key to add more.';
  MACS_LABEL: string = 'MAC Addresses';
  MACS_ERRORMSG: string = 'Invalid MAC addresses. It should be aaaa.bbbb.cccc format.';
  MACS_TOOLTIP: string = 'Type in mac address and hit enter or space key to add more.';
  PROTS_ERRORMSG: string = 'Invalid Protocol/Port';
  PROTS_TOOLTIP: string = 'Type in valid layer3 or layer 4 protocol and protocol/port, ' +
    'hit enter or space key to add more. Port can be individual or range.' +
    'for example: icmp, any/2345, tcp/60001-60100...';

  createButtonTooltip: string = '';
  minDate: Date = new Date();
  defaultDate: Date = null;

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
    { label: 'PROTO-PORTS', value: this.PROTO_PORTS_OPTION },
    { label: 'APPS', value: this.APPS_OPTION },
  ];

  securityAppsEventUtility: HttpEventUtility<SecurityApp>;
  securityApps: ReadonlyArray<SecurityApp> = [];
  securityAppOptions: SelectItem[] = [];

  labelMatchCount: number = 0;
  repeaterReady: boolean = false;
  matchMap: Map<string, any> = new Map<string, any>();
  allNIList: string[] = [];
  radioSelection: string = 'rules';

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    protected securityService: SecurityService,
    protected uiconfigsService: UIConfigsService,
    protected networkService: NetworkService,
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
    this.getNILabels();

    // change the hour seconds to 0 and 0 to avoid confusion of local vs utc
    // seems backend has iisue, seconds can not be 0, otherwise the schdedule
    // time will become null on the backend.
    this.defaultDate = new Date();
    Utility.clearHourMinuteSecond(this.defaultDate);

    // currently backend does not support any drop packets
    // UI temporarily drop those choices.
    // once they are supported, pls uncomment out the next lines
    this.packetFilterOptions = this.packetFilterOptions.filter(item =>
      item.value === MonitoringMirrorSessionSpec_packet_filters['all-packets']
    );

    if (this.isInline) {

      const interfaceObj = this.newObject.$formGroup.get(['spec', 'interfaces']).value;
      if (interfaceObj && interfaceObj.selectors && interfaceObj.selectors.length > 0) {
        this.radioSelection = 'labels';
      } else {
        this.radioSelection = 'rules';
      }

      // conver date from staring to Date Object
      const dateValue = this.newObject.$formGroup.get(['spec', 'start-condition', 'schedule-time']).value;
      if (dateValue) {
        // need to convert utc time to local time to show it on browser
        const localDate = new Date(dateValue);
        localDate.setTime(localDate.getTime() + this.minDate.getTimezoneOffset() * 60000);
        this.newObject.$formGroup.get(['spec', 'start-condition', 'schedule-time']).setValue(localDate);
      }

      // process interface selectors
      if (this.objectData.spec['interfaces'] && this.objectData.spec['interfaces']['selectors'].length) {
        const searchExpressons: ILabelsRequirement[] =
          this.objectData.spec['interfaces']['selectors'][0].requirements;
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

    this.newObject.$formGroup.get(['spec', 'packet-size']).setValidators([minValueValidator(0)]);

    // due to currently backend does not support all drops, comment out next lines
    /*
    this.newObject.$formGroup.get(['spec', 'packet-filters']).setValidators([
      Validators.required,
      this.packetFiltersValidator()]);
    */

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

  // due to currently backend does not support all drops, comment out next lines
  /*
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
  */



  isFormValid(): boolean {
    // vs-1021 packet-filters can be empty
    // due to currently backend does not support all drops, comment out next lines
    /*
    if (!this.newObject.$formGroup.get(['spec', 'packet-filters']).valid) {
      this.createButtonTooltip = PACKET_FILTERS_ERRORMSG;
      return false;
    }
    */

    if (!this.newObject.$formGroup.get(['spec', 'packet-size']).valid) {
      this.createButtonTooltip = 'Invalid Packet Size';
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

    if (this.radioSelection === 'labels') {
      // validate interface selector
      if (this.labelMatchCount === 0) {
        this.createButtonTooltip = 'Selected Labels should match atleast one Network Interface.';
        return false;
      }

      for (let i = 0; i < this.labelOutput.length ; i++) {
        const obj = this.labelOutput[i];
        if ( !(obj.key) || !(obj.values) || !(obj.operators)) {
          this.createButtonTooltip = 'Atleast one label is incomplete.';
          return false;
        }
      }

    } else {
      // validate rules
      if (this.areAllRulesEmpty()) {
        this.createButtonTooltip = 'At least one match rule must be specified.';
        return false;
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
    if (!this.newObject.$formGroup.get(['meta', 'name']).valid) {
      return 'Error: Name field is invalid.';
    }
    return this.createButtonTooltip;
  }

  getObjectValues() {
    const currValue: IMonitoringMirrorSession = this.newObject.getFormGroupValues();
    if (currValue.spec['start-condition'] && currValue.spec['start-condition']['schedule-time']) {
      const scheduleTime: Date = currValue.spec['start-condition']['schedule-time'];
      // whatever showed on browser is actually local time, we have do magic to conver it
      // to real utc time to send to the backend
      scheduleTime.setTime(scheduleTime.getTime() - scheduleTime.getTimezoneOffset() * 60000);
      // set seconds to 30 is because backend issue.
      // if the seconds are 0, the schedule time will become 0
      scheduleTime.setSeconds(30);
    }
    currValue.spec['match-rules'] = this.rules.map(r => {
      return r.data.rule.getFormGroupValues();
    });
    const repeaterSearchExpression = this.convertFormArrayToSearchExpression(this.labelOutput);
    currValue.spec['interfaces'] = new MonitoringInterfaceMirror();
    currValue.spec['interfaces']['selectors'] = new Array<LabelsSelector>();
    currValue.spec['interfaces']['selectors'].push(new LabelsSelector());
    currValue.spec['interfaces']['selectors'][0].requirements = repeaterSearchExpression as ILabelsRequirement[];

    if (this.radioSelection === 'labels') {
      currValue.spec['match-rules'] = [];
      currValue.spec['packet-filters'] = [];
    } else {
      currValue.spec['interfaces'] = null;
    }

    return currValue;
  }

  convertFormArrayToSearchExpression(value, addMetatag: boolean = false) {
    const data = value;
    if (data == null) {
      return null;
    }

    let retData = data.filter((item) => {
      return !Utility.isEmpty(item.key) && !Utility.isEmpty(item.values) && item.values.length !== 0;
    });
    // make sure the value field is an array
    retData = retData.map((item) => {
      const tag = item.key;
      const keyValue = ((addMetatag) ? 'meta.labels.' : '')  + tag;
      // modify here to trim each string of the array
      const valValues = Array.isArray(item.values) ?
        item.values : item.values.trim().split(',');
      const trimmedValues = valValues.map(each => (each) ? each.trim() : each);
      const searchExpression: SearchExpression = {
        key:  keyValue,
        operator: item.operators,
        values: trimmedValues
      };
      return searchExpression;
    });
    return retData;
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
    const matches = this.getMatches(values);
    this.labelMatchCount = matches.length;
    this.labelOutput = values;
  }

  addCollector() {
    const collectors = this.newObject.$formGroup.get(['spec', 'collectors']) as FormArray;
    const newCollector = new MonitoringMirrorCollector().$formGroup;
    newCollector.get(['export-config', 'destination']).setValidators([
      newCollector.get(['export-config', 'destination']).validator, IPUtility.isValidIPValidator]);
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
    // due to currently backend does not support all drops, comment out next lines
    /*
    rule.$formGroup.valueChanges.subscribe(() => {
      this.newObject.$formGroup.get(['spec', 'packet-filters']).updateValueAndValidity();
    });
    */
    this.rules.push({
      id: Utility.s4(),
      data: { rule: rule, selectedProtoAppOption: this.PROTO_PORTS_OPTION },
      inEdit: false
    });
    this.editRule(this.rules.length - 1);
  }

  editRule(index) {
    // Collapse any other open rules, and make index rule open
    this.rules.forEach((r, i) => {
      if (i === index) {
        r.inEdit = true;
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
    if (arr.length > 2) {
      return false;
    }
    if (arr.length === 1) {
      return prot === 'any' || prot === 'icmp';
    }
    if (prot !== 'tcp' && prot !== 'udp' && prot !== 'any') {
      return false;
    }
    return Utility.isPortRangeValid(arr[1]);
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

  isShowAddCollector(): boolean {
    const collectors = this.newObject.$formGroup.get(['spec', 'collectors']) as FormArray;
    if (collectors.length < 4) {
      return true;
    }
    return false;
  }

  getMatches(selectors) {
    let matchList = [];
    let first: boolean = true;
    for (const x of selectors) {
      if (x['key'] && x['operators'] && x['values']) {
        const matches = this.getRuleMatches(x);
        if (first) {
          matchList = matches;
          first = false;
        } else {
          matchList = [...matchList].filter(i => matches.includes(i));
        }
      }
    }
    return matchList;
  }

  getRuleMatches(rule): any {
    const key = rule['key'];
    const op = rule['operators'];
    const val = rule['values'];
    let matches = [];
    if (this.matchMap[key] != null && this.matchMap[key][val] != null && this.matchMap[key][val].length !== 0) {
      matches = this.matchMap[key][val];
    }

    if (op === 'in') {
      return matches;
    } else {
      return [...this.allNIList].filter(i => !matches.includes(i));
    }
  }

  getNILabels() {
    const sub = this.networkService.ListNetworkInterfacesCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        const body: NetworkNetworkInterface[] = response.data as NetworkNetworkInterface[];
        this.matchMap = new Map<string, any>();
        this.labelData = [];
        this.allNIList = [];
        this.buildLabelInformation(body);
        this.buildLabelInformationFromMirrorSessions();
        this.buildLabelData();
      }
    );
    this.subscriptions.push(sub);
  }

  buildLabelInformationFromMirrorSessions() {
    for (const obj of this.existingObjects) {
      if (obj.spec.interfaces && obj.spec.interfaces.selectors.length && obj.spec.interfaces.selectors[0]) {
        for (const i of obj.spec.interfaces.selectors[0].requirements ) {
          const key = i.key;
          const values = i.values;
          if (this.matchMap[key] == null) {
            console.log('key doesnt exist anymore');
            this.matchMap[key] = new Map<string, any>();
          }
          for (const v of values) {
            if (this.matchMap[key][v] == null) {
              console.log('Key value pair doesnt exist anymore');
              this.matchMap[key][v] = [];
            }
          }
        }
      }
    }
  }

  buildLabelInformation(networkInterfaces: NetworkNetworkInterface[]) {

    if (networkInterfaces.length) {
      networkInterfaces.forEach((item: NetworkNetworkInterface) => {
        if (item.meta && item.meta.labels && item.spec.type === NetworkNetworkInterfaceSpec_type['uplink-eth']) {
          for (const key of Object.keys(item.meta.labels)) {
            if (this.matchMap[key] == null) {
              this.matchMap[key] = new Map<string, any>();
            }
            if (this.matchMap[key][item.meta.labels[key]] == null) {
              this.matchMap[key][item.meta.labels[key]] = [];
            }
            this.matchMap[key][item.meta.labels[key]].push(item.meta.uuid);
          }
        }
        this.allNIList.push(item.meta.uuid);
      });
    }
  }

  buildLabelData() {
    for (const key of Object.keys(this.matchMap)) {
      const valList = [];
      for (const val of Object.keys(this.matchMap[key])) {
        valList.push({ value: val, label: val });
      }

      const label = {
        key: { value: key, label: key },
        values: valList,
        operators: SearchUtil.stringOperators,
        fieldType: ValueType.singleSelect,
        valueType: ValueType.multiSelect,
        keyLabel: 'Label Key',
        valueLabel: 'Label Value'
      };
      this.labelData.push(label);
    }
    this.repeaterReady = true;
  }

  buildKeyPlaceholder(repeater: any, keyFormName: string): string {
    return 'key';
  }

  buildValuePlaceholder(repeater: any, keyFormName: string): string {
    return 'value';
  }

}
