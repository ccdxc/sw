import { Component, OnInit, ViewEncapsulation, Input } from '@angular/core';
import { Animations } from '@app/animations';
import { MonitoringMatchRule, IMonitoringMatchRule, MonitoringFlowExportPolicySpec } from '@sdk/v1/models/generated/monitoring';
import { FormArray, FormGroup } from '@angular/forms';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { Utility } from '@app/common/Utility';

@Component({
  selector: 'app-matchrule',
  templateUrl: './matchrule.component.html',
  styleUrls: ['./matchrule.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class MatchruleComponent extends BaseComponent implements OnInit {
  @Input() rules: IMonitoringMatchRule[] = [];
  @Input() customizeValidFunc: () => boolean = null;

  flowTarget: FormGroup;
  matchRules: FormGroup[];

  constructor(controllerService: ControllerService) {
    super(controllerService, null);
   }

  ngOnInit() {
    let flowTarget;
    if (this.rules != null) {
      flowTarget = new MonitoringFlowExportPolicySpec({ 'match-rules': this.rules });
    } else {
      flowTarget = new MonitoringFlowExportPolicySpec();
    }
    this.flowTarget = flowTarget.$formGroup;
    const matchRules: any = this.flowTarget.get('match-rules');
    if (matchRules.controls.length === 0) {
      this.addMatchRule();
    }
    this.matchRules = (<any>this.flowTarget.get(['match-rules'])).controls;
  }

  getValues(): IMonitoringMatchRule[] {
    const ret = this.flowTarget.value['match-rules'];
    ret.forEach((flow) => {
      for (const k in flow.source) {
        if (flow.source.hasOwnProperty(k)) {
          const v: string = flow.source[k];
          if (v != null && typeof v === 'string') {
            flow.source[k] = v.split(',')
              .map((item: string) => item.trim())
              .filter((item: string) => item.length > 0);
          }
        }
      }
      for (const k in flow.destination) {
        if (flow.destination.hasOwnProperty(k)) {
          const v: string = flow.destination[k];
          if (v != null && typeof v === 'string') {
            flow.destination[k] = v.split(',')
              .map((item: string) => item.trim())
              .filter((item: string) => item.length > 0);
          }
        }
      }
      for (const k in flow['app-protocol-selectors']) {
        if (flow['app-protocol-selectors'].hasOwnProperty(k)) {
          const v: string = flow['app-protocol-selectors'][k];
          if (v != null && typeof v === 'string') {
            flow['app-protocol-selectors'][k] = v.split(',')
              .map((item: string) => item.trim())
              .filter((item: string) => item.length > 0);
          }
        }
      }
    });
    return ret;
  }

  addMatchRule() {
    const matchRules = this.flowTarget.get('match-rules') as FormArray;
    matchRules.insert(0, new MonitoringMatchRule().$formGroup);
  }

  deleteMatchRule(index) {
    const matchRules = this.flowTarget.get('match-rules') as FormArray;
    if (matchRules.length > 1) {
      matchRules.removeAt(index);
    }
  }

  isValid(): boolean {
     if (! this.customizeValidFunc) {
       return this.validateAllRulesEmpty();
     } else {
       return this.customizeValidFunc();
     }
  }

  /**
   * Check wheter all rules are empty
   */
  validateAllRulesEmpty(): boolean {
    let rules = this.getValues();
    rules = Utility.TrimDefaultsAndEmptyFields(rules);
    let countEmptyRule: number = 0;
    for (let i = 0; i < rules.length; i++ ) {
      if (!rules[i]) {
        countEmptyRule  += 1;
      }
    }
    return (countEmptyRule === rules.length);
  }



}
