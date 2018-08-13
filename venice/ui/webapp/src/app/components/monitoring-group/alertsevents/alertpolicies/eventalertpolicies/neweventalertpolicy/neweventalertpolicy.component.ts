import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { ErrorStateMatcher } from '@angular/material';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringAlertDestination, IMonitoringAlertPolicy, MonitoringAlertPolicy, MonitoringAlertPolicySpec, MonitoringAlertStatus } from '@sdk/v1/models/generated/monitoring';
import { SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs';
import { RepeaterData, ValueType } from 'web-app-framework';
import { Validators } from '@angular/forms';
import { SearchSearchQuery_kinds } from '@sdk/v1/models/generated/search';

@Component({
  selector: 'app-neweventalertpolicy',
  templateUrl: './neweventalertpolicy.component.html',
  styleUrls: ['./neweventalertpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NeweventalertpolicyComponent implements OnInit, AfterViewInit {
  newPolicy: MonitoringAlertPolicy;

  @Input() isInline: boolean = false;
  @Input() policyData: any;
  @Input() destinations: IMonitoringAlertDestination[] = [];
  @Output() formClose: EventEmitter<any> = new EventEmitter();


  repeaterOptions: RepeaterData[] = [
    {
      key: { label: 'severity', value: 'severity' },
      operators: [
        { label: 'Equals', value: 'Equals' },
        { label: 'NotEquals', value: 'NotEquals' }
      ],
      values: Utility.convertEnumToSelectItem(MonitoringAlertStatus.propInfo['severity'].enum),
      valueType: ValueType.multiSelect
    },
    {
      key: { label: 'kind', value: 'kind' },
      operators: [
        { label: 'Equals', value: 'Equals' },
        { label: 'NotEquals', value: 'NotEquals' }
      ],
      values: Utility.convertEnumToSelectItem(SearchSearchQuery_kinds),
      valueType: ValueType.multiSelect
    },
    {
      key: { label: 'name', value: 'name' },
      operators: [
        { label: 'Equals', value: 'Equals' },
        { label: 'NotEquals', value: 'NotEquals' }
      ],
      valueType: ValueType.inputField
    },
  ];

  alertOptions: SelectItem[] = Utility.convertEnumToSelectItem(MonitoringAlertPolicySpec.propInfo['severity'].enum);

  destinationOptions: SelectItem[] = [];

  oldButtons: ToolbarButton[] = [];

  errorChecker = new ErrorStateMatcher();

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService
  ) { }

  ngOnInit() {
    if (this.policyData != null) {
      this.newPolicy = new MonitoringAlertPolicy(this.policyData);
    } else {
      this.newPolicy = new MonitoringAlertPolicy();
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
      })
    })

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
          callback: () => { this.savePolicy() },
          computeClass: () => { return this.computeButtonClass() }
        },
        {
          cssClass: 'global-button-neutral eventalertpolicies-button',
          text: 'CANCEL',
          callback: () => { this.cancelPolicy() }
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
    let handler: Observable<{ body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number }>;
    if (this.isInline) {
      handler = this._monitoringService.UpdateAlertPolicy(this.newPolicy.meta.name, this.newPolicy);
    } else {
      handler = this._monitoringService.AddAlertPolicy(this.newPolicy);
    }

    handler.subscribe((response) => {
      const status = response.statusCode;
      if (status === 200) {
        if (!this.isInline) {
          // Need to reset the toolbar that we changed
          this.setPreviousToolbar();
        }
        this.formClose.emit();
      } else {
        console.log(response.body);
      }
    })
  }

  cancelPolicy() {
    if (!this.isInline) {
      // Need to reset the toolbar that we changed
      this.setPreviousToolbar();
    }
    this.formClose.emit();
  }
}
