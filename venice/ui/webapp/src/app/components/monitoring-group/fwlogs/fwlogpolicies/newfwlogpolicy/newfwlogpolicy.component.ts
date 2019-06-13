import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild } from '@angular/core';
import { Animations } from '@app/animations';
import { BaseComponent } from '@app/components/base/base.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringFwlogPolicy, MonitoringFwlogPolicy, MonitoringFwlogPolicySpec } from '@sdk/v1/models/generated/monitoring';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { SyslogComponent } from '@app/components/monitoring-group/syslog/syslog.component';
import { Utility } from '@app/common/Utility';
import { required } from '@sdk/v1/utils/validators';

@Component({
  selector: 'app-newfwlogpolicy',
  templateUrl: './newfwlogpolicy.component.html',
  styleUrls: ['./newfwlogpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NewfwlogpolicyComponent extends BaseComponent implements OnInit, AfterViewInit {
  public static LOGOPTIONS_ALL = 'FIREWALL_ACTION_ALL';
  public static LOGOPTIONS_NONE = 'FIREWALL_ACTION_NONE';

  // @ViewChild('fieldSelector') fieldSelector: FieldselectorComponent;
  @ViewChild('syslogComponent') syslogComponent: SyslogComponent;
  @ViewChild('logOptions') logOptionsMultiSelect: MultiSelect;

  newPolicy: MonitoringFwlogPolicy;

  @Input() isInline: boolean = false;
  @Input() policyData: IMonitoringFwlogPolicy;
  @Input() maxTargets: number;
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  filterOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringFwlogPolicySpec.propInfo['filter'].enum, [NewfwlogpolicyComponent.LOGOPTIONS_NONE]);

  oldButtons: ToolbarButton[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    if (this.policyData != null) {
      this.newPolicy = new MonitoringFwlogPolicy(this.policyData);
    } else {
      this.newPolicy = new MonitoringFwlogPolicy();
    }

    if (this.isInline) {
      // disable name field
      this.newPolicy.$formGroup.get(['meta', 'name']).disable();
    }
  }

  ngAfterViewInit() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary fwlogpolicies-button fwlogpolicies-button-SAVE',
          text: 'CREATE FIREWALL LOG POLICY',
          callback: () => { this.savePolicy(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-neutral fwlogpolicies-button fwlogpolicies-button-CANCEL',
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
    const policy: IMonitoringFwlogPolicy = this.newPolicy.getFormGroupValues();
    let handler: Observable<{ body: IMonitoringFwlogPolicy | IApiStatus | Error, statusCode: number }>;

    const syslogValues = this.syslogComponent.getValues();
    Object.keys(syslogValues).forEach((key) => {
      policy.spec[key] = syslogValues[key];
    });

    if (this.isInline) {
      // Using this.newPolicy to get name, as the name is gone when we call getFormGroupValues
      // This is beacuse we disabled it in the form group to stop the user from editing it.
      // When you disable an angular control, in doesn't show up when you get the value of the group
      handler = this._monitoringService.UpdateFwlogPolicy(this.newPolicy.meta.name, policy);
    } else {
      handler = this._monitoringService.AddFwlogPolicy(policy);
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

  /**
   * event.itemValue is the changed item. event.value has the selections
   * @param event
   */
  /* VS-124.
     When there is any NONE-ALL options selected, OPTION_ALL will be removed
     When none is seleced, we set OPTION_ALL as default.
  */
  onLogOptionChange(event) {
    const values = this.logOptionsMultiSelect.value;
    if (values.length >= 1 && values.includes(NewfwlogpolicyComponent.LOGOPTIONS_ALL)) {
      // When all is set, we untoggle everything else.
      if (event.itemValue === NewfwlogpolicyComponent.LOGOPTIONS_ALL) {
        this.logOptionsMultiSelect.value = [NewfwlogpolicyComponent.LOGOPTIONS_ALL];
      } else {
        const index = values.indexOf(NewfwlogpolicyComponent.LOGOPTIONS_ALL);
        values.splice(index, 1);
        this.logOptionsMultiSelect.value = values;
      }
    }
  }
}
