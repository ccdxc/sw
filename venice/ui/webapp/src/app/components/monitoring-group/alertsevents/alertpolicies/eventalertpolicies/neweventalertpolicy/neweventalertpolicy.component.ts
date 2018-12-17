import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewChild, ViewEncapsulation } from '@angular/core';
import { Validators } from '@angular/forms';
import { ErrorStateMatcher } from '@angular/material';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringAlertDestination, IMonitoringAlertPolicy, MonitoringAlertPolicy, MonitoringAlertPolicySpec } from '@sdk/v1/models/generated/monitoring';
import { MessageService, SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs';

@Component({
  selector: 'app-neweventalertpolicy',
  templateUrl: './neweventalertpolicy.component.html',
  styleUrls: ['./neweventalertpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NeweventalertpolicyComponent extends BaseComponent implements OnInit, AfterViewInit {
  @ViewChild('fieldSelector') fieldSelector: FieldselectorComponent;
  newPolicy: MonitoringAlertPolicy;

  @Input() isInline: boolean = false;
  @Input() policyData: IMonitoringAlertPolicy;
  @Input() destinations: IMonitoringAlertDestination[] = [];
  @Output() formClose: EventEmitter<any> = new EventEmitter();

  alertOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringAlertPolicySpec.propInfo['severity'].enum);

  destinationOptions: SelectItem[] = [];

  oldButtons: ToolbarButton[] = [];

  errorChecker = new ErrorStateMatcher();


  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    if (this.policyData != null) {
      this.newPolicy = new MonitoringAlertPolicy(this.policyData);
    } else {
      this.newPolicy = new MonitoringAlertPolicy();
      // Remove once Sanjay adds better defaults to the swagger
      this.newPolicy.spec.enable = true;
    }

    if (this.isInline) {
      // disable name field
      this.newPolicy.$formGroup.get(['meta', 'name']).disable();
    } else {
      // Name field can't be blank
      this.newPolicy.$formGroup.get(['meta', 'name']).setValidators(Validators.required);
    }
    this.newPolicy.$formGroup.get(['spec', 'resource']).setValue('Event');


    this.destinations.forEach((destination) => {
      this.destinationOptions.push({
        label: destination.meta.name,
        value: destination.meta.name,
      });
    });
  }

  isErrorState(control) {
    return this.errorChecker.isErrorState(control, null);
  }

  ngAfterViewInit() {
    if (!this.isInline) {
      // If it is not inline, we change the toolbar buttons, and save the old one
      // so that we can set it back when we are done
      const currToolbar = this._controllerService.getToolbarData();
      this.oldButtons = currToolbar.buttons;
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary eventalertpolicies-button',
          text: 'CREATE ALERT POLICY',
          callback: () => { this.savePolicy(); },
          computeClass: () => this.computeButtonClass()
        },
        {
          cssClass: 'global-button-neutral eventalertpolicies-button',
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
    const policy: IMonitoringAlertPolicy = this.newPolicy.getFormGroupValues();
    let handler: Observable<{ body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number }>;

    policy.spec.requirements = this.fieldSelector.getValues();
    if (this.isInline) {
      // Using this.newPolicy to get name, as the name is gone when we call getFormGroupValues
      // This is beacuse we disabled it in the form group to stop the user from editing it.
      // When you disable an angular control, in doesn't show up when you get the value of the group
      handler = this._monitoringService.UpdateAlertPolicy(this.newPolicy.meta.name, policy);
    } else {
      handler = this._monitoringService.AddAlertPolicy(policy);
    }

    handler.subscribe(
      (response) => {
        if (this.isInline) {
          this.invokeSuccessToaster('Update Successful', 'Updated policy ' + this.newPolicy.meta.name);
        } else {
          this.invokeSuccessToaster('Creation Successful', 'Created policy ' + policy.meta.name);
        }
        this.cancelPolicy();
      },
      (error) => {
        if (this.isInline) {
          this.invokeRESTErrorToaster('Update Failed', error);
        } else {
          this.invokeRESTErrorToaster('Creation Failed', error);
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
