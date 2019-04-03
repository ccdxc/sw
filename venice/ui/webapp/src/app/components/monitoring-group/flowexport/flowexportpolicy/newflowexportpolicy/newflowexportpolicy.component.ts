import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild } from '@angular/core';
import { Animations } from '@app/animations';
import { BaseComponent } from '@app/components/base/base.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, MonitoringFlowExportPolicy, IMonitoringFlowExportPolicy, IMonitoringSyslogExport, MonitoringFlowExportPolicySpec } from '@sdk/v1/models/generated/monitoring';
import { SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { SyslogComponent } from '@app/components/monitoring-group/syslog/syslog.component';
import { MatchruleComponent } from '@app/components/monitoring-group/matchrule/matchrule.component';
import { Utility } from '@app/common/Utility';
import { required } from '@sdk/v1/utils/validators';

@Component({
  selector: 'app-newflowexportpolicy',
  templateUrl: './newflowexportpolicy.component.html',
  styleUrls: ['./newflowexportpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NewflowexportpolicyComponent extends BaseComponent implements OnInit, AfterViewInit {
  @ViewChild('matchRules') matchRulesComponent: MatchruleComponent;
  @ViewChild('syslogComponent') syslogComponent: SyslogComponent;
  newPolicy: MonitoringFlowExportPolicy;

  @Input() isInline: boolean = false;
  @Input() policyData: IMonitoringFlowExportPolicy;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  syslogConfig: IMonitoringSyslogExport;

  formatOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringFlowExportPolicySpec.propInfo['format'].enum);

  oldButtons: ToolbarButton[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    if (this.policyData != null) {
      this.newPolicy = new MonitoringFlowExportPolicy(this.policyData);
    } else {
      this.newPolicy = new MonitoringFlowExportPolicy();
    }

    if (this.isInline) {
      // disable name field
      this.newPolicy.$formGroup.get(['meta', 'name']).disable();
    }
    // Field is labeled as exports for this object instead of target
    this.syslogConfig = {
      format: this.newPolicy.spec.format as any,
      targets: this.newPolicy.spec.exports,
    };
  }

  ngAfterViewInit() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary flowexportpolicy-button',
          text: 'CREATE FLOW EXPORT POLICY',
          callback: () => { this.savePolicy(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-neutral flowexportpolicy-button',
          text: 'CANCEL',
          callback: () => { this.cancelPolicy(); }
        },
      ];

      this._controllerService.setToolbarData(currToolbar);
    }
  }

  computeButtonClass() {
    if (this.newPolicy.$formGroup.get('meta.name').status === 'VALID') {
      return '';
    } else {
      return 'global-button-disabled';
    }
  }

  /**
   * Sets the previously saved toolbar buttons
   * They should have been saved in the ngOnInit when we are inline.
   */
  setPreviousToolbar() {
    if (this.oldButtons != null) {
      const currToolbar = this._controllerService.getToolbarData();
      currToolbar.buttons = this.oldButtons;
      this._controllerService.setToolbarData(currToolbar);
    }
  }

  savePolicy() {
    // Submit to server
    const policy: IMonitoringFlowExportPolicy = this.newPolicy.getFormGroupValues();
    let handler: Observable<{ body: MonitoringFlowExportPolicy | IApiStatus | Error, statusCode: number }>;

    policy.spec['match-rules'] = this.matchRulesComponent.getValues();

    const syslogValues = this.syslogComponent.getValues();
    Object.keys(syslogValues).forEach((key) => {
      if (key === 'targets') {
        policy.spec['exports'] = syslogValues[key];
      } else {
        policy.spec[key] = syslogValues[key];
      }
    });

    if (this.isInline) {
      // Using this.newPolicy to get name, as the name is gone when we call getFormGroupValues
      // This is beacuse we disabled it in the form group to stop the user from editing it.
      // When you disable an angular control, in doesn't show up when you get the value of the group
      handler = this._monitoringService.UpdateFlowExportPolicy(this.newPolicy.meta.name, policy);
    } else {
      handler = this._monitoringService.AddFlowExportPolicy(policy);
    }

    handler.subscribe(
      (response) => {
        if (this.isInline) {
          this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Updated policy ' + this.newPolicy.meta.name);
        } else {
          this._controllerService.invokeSuccessToaster(Utility.CREATE_SUCCESS_SUMMARY, 'Created policy ' + policy.meta.name);
        }
        this.cancelPolicy();
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

  cancelPolicy() {
    if (!this.isInline) {
      // Need to reset the toolbar that we changed
      this.setPreviousToolbar();
    }
    this.formClose.emit();
  }
}
