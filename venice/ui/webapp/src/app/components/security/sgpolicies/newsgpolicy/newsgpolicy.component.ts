import { Component, OnInit, ViewEncapsulation, AfterViewInit, Input, Output, EventEmitter } from '@angular/core';
import { ISecurityNetworkSecurityPolicy, SecurityNetworkSecurityPolicy, SecuritySGRule, SecurityProtoPort, SecurityApp, SecuritySecurityGroup, ISecuritySecurityGroup } from '@sdk/v1/models/generated/security';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Animations } from '@app/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { OrderedItem } from '@app/components/shared/orderedlist/orderedlist.component';
import { Utility } from '@app/common/Utility';
import { SelectItem } from 'primeng/api';
import { IPUtility } from '@app/common/IPUtility';
import { FormArray, FormControl, ValidatorFn } from '@angular/forms';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';

@Component({
  selector: 'app-newsgpolicy',
  templateUrl: './newsgpolicy.component.html',
  styleUrls: ['./newsgpolicy.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
})
export class NewsgpolicyComponent extends CreationForm<ISecurityNetworkSecurityPolicy, SecurityNetworkSecurityPolicy> implements OnInit, AfterViewInit {
  // TODO: Add validation
  // TODO: either prevent adding new rule while one is incomplete,
  //       or add indicator that a rule is invalid/incomplete


  ATTACH_TENANT: string = 'tenant';
  ATTACH_SG: string = 'securityGroups';

  PROTO_PORTS_OPTION: string = 'proto-ports';
  APPS_OPTION: string = 'apps';


  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected securityService: SecurityService,
    protected workloadService: WorkloadService,
  ) {
    super(_controllerService, uiconfigsService, SecurityNetworkSecurityPolicy);
  }

  attachOptions = [
    { label: 'TENANT', value: this.ATTACH_TENANT },
    // Product Release-A constraints
    // { label: 'SECURITY GROUPS', value: this.ATTACH_SG },
  ];

  selectedAttachOption: string = this.ATTACH_TENANT;

  fakeSecurityGroups = [
    { label: 'security-group-1', value: 'security-group-1' },
    { label: 'security-group-2', value: 'security-group-2' },
    { label: 'security-group-3', value: 'security-group-3' },
  ];

  fakeApps = [
    { label: 'app-1', value: 'security-group-1' },
    { label: 'app-2', value: 'security-group-2' },
    { label: 'app-3', value: 'security-group-3' },
  ];

  protoAppOptions = [
    { label: 'PROTO-PORTS', value: this.PROTO_PORTS_OPTION },
    { label: 'APPS', value: this.APPS_OPTION },
  ];

  selectedProtoAppOption: string = this.PROTO_PORTS_OPTION;

  repeaterAnimationEnabled = true;

  actionEnum = SecuritySGRule.propInfo.action.enum;
  actionOptions: SelectItem[] = Utility.convertEnumToSelectItem(SecuritySGRule.propInfo.action.enum);

  rules: OrderedItem<SecuritySGRule>[] = [];

  sgpolicyOptions;

  // TODO: must be a number
  // TODO: must be required
  ruleNumberEditControl = new FormControl('');

  // Angular has an issue with stuttering the animation
  // for collapsing (exit animation) after a drag and drop event
  // To smooth the experience, we disable animation on that index only
  indexToSkipAnimation: number;

  securityAppsEventUtility: HttpEventUtility<SecurityApp>;
  securityApps: ReadonlyArray<SecurityApp> = [];
  securityAppOptions: SelectItem[] = [];
  securityGroupsEventUtility: HttpEventUtility<SecuritySecurityGroup>;
  securityGroups: ReadonlyArray<SecuritySecurityGroup> = [];
  securityGroupOptions: SelectItem[] = [];
  workloadEventUtility: HttpEventUtility<WorkloadWorkload>;
  workloads: ReadonlyArray<WorkloadWorkload> = [];

  // Map from IP to workload name
  ipOptions: any[] = [];

  @Input() isInline: boolean = false;
  @Input() existingObjects: ISecurityNetworkSecurityPolicy[] = [];
  @Input() objectData: ISecurityNetworkSecurityPolicy;
  @Output() formClose: EventEmitter<any> = new EventEmitter();


  getClassName() {
    return this.constructor.name;
  }

  // Empty Hook
  postNgInit() {
    this.getSecurityApps();
    this.getWorkloads();
    this.getSecuritygroups();
    if (!this.isInline) {
      if (this.rules.length === 0) {
        this.addRule();
      }
    } else {
      // TODO: comment out me and implement editing policy
      this.objectData.spec.rules.forEach((rule: SecuritySGRule) => {
        const uiIRule = new SecuritySGRule(rule);

        this.rules.push({
          id: Utility.s4(),
          data: uiIRule,
          inEdit: false,
        });
      });
    }

    this.setValidationRules();
    // Add a default protocol port if its the default option
    // if (this.selectedProtoAppOption === this.PROTO_PORTS_OPTION)
  }

  setValidationRules() {
    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
      this.isNewNetworkSecurityPolicyNameValid(this.existingObjects)]);
  }

  isNewNetworkSecurityPolicyNameValid(existingObjects: ISecurityNetworkSecurityPolicy[]): ValidatorFn {
    // checks if name field is valid
    return Utility.isModelNameUniqueValidator(existingObjects, 'newPolicy-name');
  }

  getWorkloads() {
    this.workloadEventUtility = new HttpEventUtility<WorkloadWorkload>(WorkloadWorkload);
    this.workloads = this.workloadEventUtility.array;
    const subscription = this.workloadService.WatchWorkload().subscribe(
      (response) => {
        this.workloadEventUtility.processEvents(response);
        this.buildIPMap();
      },
      this._controllerService.webSocketErrorHandler('Failed to get Workloads')
    );
    this.subscriptions.push(subscription);
  }

  buildIPMap() {
    const ipMap = {};
    this.ipOptions = [];
    // Taking IPs from spec, since status isn't always filled out currently
    // TODO: Take IPs from status
    this.workloads.forEach((w) => {
      w.spec.interfaces.forEach((intf) => {
        intf['ip-addresses'].forEach((ip) => {
          ipMap[ip] = w.meta.name;
        });
      });
    });
    Object.keys(ipMap).forEach(ip => {
      this.ipOptions.push({ ip: ip, workload: ipMap[ip] });
    });
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
            value: app.meta.name,
          };
        });
      },
      this._controllerService.webSocketErrorHandler('Failed to get Apps')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  getSecuritygroups() {
    this.securityGroupsEventUtility = new HttpEventUtility<SecuritySecurityGroup>(SecuritySecurityGroup, true);
    this.securityGroups = this.securityGroupsEventUtility.array as ReadonlyArray<SecuritySecurityGroup>;
    const subscription = this.securityService.WatchSecurityGroup().subscribe(
      response => {
        this.securityGroupsEventUtility.processEvents(response);
        this.securityGroupOptions = this.securityGroups.map(group => {
          return {
            label: group.meta.name,
            value: group.meta.name,
          };
        });
      },
      this.controllerService.webSocketErrorHandler('Failed to get Security Groups info')
    );
    this.subscriptions.push(subscription);
  }

  // Empty Hook
  isFormValid() {
    if (!Utility.isEmpty(this.newObject.$formGroup.get('meta.name').value)
      && this.newObject.$formGroup.valid
    ) {
      return true;
    }
    return false;
  }

  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary global-button-padding',
        text: 'CREATE POLICY',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass()
      },
      {
        cssClass: 'global-button-neutral global-button-padding',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  createObject(object: ISecurityNetworkSecurityPolicy) {
    return this.securityService.AddNetworkSecurityPolicy(object);
  }

  updateObject(newObject: ISecurityNetworkSecurityPolicy, oldObject: ISecurityNetworkSecurityPolicy) {
    return this.securityService.UpdateNetworkSecurityPolicy(oldObject.meta.name, newObject, null, oldObject);
  }

  generateCreateSuccessMsg(object: ISecurityNetworkSecurityPolicy) {
    return 'Created policy ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: ISecurityNetworkSecurityPolicy) {
    return 'Updated policy ' + object.meta.name;
  }

  getObjectValues() {
    const currValue: ISecurityNetworkSecurityPolicy = this.newObject.getFormGroupValues();
    if (this.selectedAttachOption === this.ATTACH_TENANT) {
      currValue.spec['attach-tenant'] = true;
    }
    currValue.spec.rules = this.rules.map(r => {
      return r.data.getFormGroupValues();
    });
    return currValue;
  }

  moveRule(start, target) {
    if (target > this.rules.length - 1) {
      target = this.rules.length - 1;
    }
    if (target === start) {
      return;
    }
    const currRule = this.rules[start].data;
    // deleting previous item
    this.rules.splice(start, 1);
    // inserting new item
    if (target > start) {
      // adjust target since length changed.
      target -= 1;
    }
    this.rules.splice(target, 0, {
      id: Utility.s4(),
      data: currRule,
      inEdit: false,
    });
  }

  editRule(index) {
    // Collapse any other open rules, and make index rule open
    this.rules.forEach((r, i) => {
        // Before we collapse rule, we clear the unused protoAppOption
        if (this.selectedProtoAppOption === this.PROTO_PORTS_OPTION) {
          r.data.$formGroup.get('apps').setValue([]);
        } else {
          const protoPort = r.data.$formGroup.get('proto-ports') as FormArray;
          while (protoPort.controls.length !== 0) {
            protoPort.removeAt(0);
          }
        }
        // If rule number has changed we move it now
        // TODO: Check if number is valid, reset it if it isn't
        if ( r.inEdit && this.ruleNumberEditControl.value !== i + 1) {
          this.moveRule(i, this.ruleNumberEditControl.value - 1);
        }
        r.inEdit = false;
    });
    this.rules.some((r, i) => {
      if (i === index) {
        r.inEdit = true;
        const appVal = r.data.$formGroup.get('apps').value;
        const hasApp = appVal != null && appVal.length > 0;
        if (hasApp) {
          this.selectedProtoAppOption = this.APPS_OPTION;
        } else {
          this.selectedProtoAppOption = this.PROTO_PORTS_OPTION;
        }
        if ((r.data.$formGroup.get('proto-ports') as FormArray).length === 0) {
          this.addProtoTarget(index);
        }
        this.ruleNumberEditControl.setValue(index + 1);
        return true;
      }
      return false;
    });

    if (this.indexToSkipAnimation != null) {
      setTimeout(() => {
        this.indexToSkipAnimation = null;
      }, 300);
    }
  }

  addRule() {
    const rule = new SecuritySGRule();
    this.rules.push({
      id: Utility.s4(),
      data: rule,
      inEdit: false,
    });
    this.editRule(this.rules.length - 1);
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

  onOrderChange() {
    // We disable animation while the re-order animation happens
    // to prevent app-repeater from stuttering
    this.repeaterAnimationEnabled = false;
    this.indexToSkipAnimation = this.rules.findIndex(x => x.inEdit);
    setTimeout(() => {
      this.repeaterAnimationEnabled = true;
    }, 500);
  }

  isValidIP(ip: string) {
    return IPUtility.isValidIP(ip);
  }

  addProtoTarget(ruleIndex: number) {
    const tempTargets = this.rules[ruleIndex].data.$formGroup.get(['proto-ports']) as FormArray;
    tempTargets.insert(0, new SecurityProtoPort().$formGroup);
  }

  removeProtoTarget(ruleIndex: number, index: number) {
    const tempTargets = this.rules[ruleIndex].data.$formGroup.get(['proto-ports']) as FormArray;
    if (tempTargets.length > 1) {
      tempTargets.removeAt(index);
    }
  }

  displayArrayField(rule, field) {
    if (field === 'proto-ports') {
      const protoPorts = rule.$formGroup.get(field).value.filter((entry) => {
        return entry.protocol != null;
      }).map((entry) => {
        if (entry.ports == null || entry.ports.length === 0) {
          return entry.protocol;
        }
        return entry.protocol + '/' + entry.ports;
      });
      return protoPorts.join(', ');
    }
    return rule.$formGroup.get(field).value.join(', ');
  }

}
