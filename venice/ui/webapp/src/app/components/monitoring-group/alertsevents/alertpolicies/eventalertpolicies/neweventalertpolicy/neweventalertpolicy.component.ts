import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewChild, ViewEncapsulation } from '@angular/core';
import { Validators, FormControl, FormGroup, FormArray } from '@angular/forms';
import { ErrorStateMatcher } from '@angular/material';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringAlertDestination, IMonitoringAlertPolicy, MonitoringAlertPolicy, MonitoringAlertPolicySpec, FieldsRequirement } from '@sdk/v1/models/generated/monitoring';
import { SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs/Observable';

@Component({
  selector: 'app-neweventalertpolicy',
  templateUrl: './neweventalertpolicy.component.html',
  styleUrls: ['./neweventalertpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class NeweventalertpolicyComponent implements OnInit, AfterViewInit {
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

  destinationsControl: FormControl;

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService
  ) { }

  ngOnInit() {
    if (this.policyData != null) {
      this.newPolicy = new MonitoringAlertPolicy(this.policyData);
    } else {
      this.newPolicy = new MonitoringAlertPolicy();
      // Remove once Sanjay adds better defaults to the swagger
      this.newPolicy.spec.enable = true;
    }
    this.destinationsControl = new FormControl(this.newPolicy.spec.destinations);

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
    // Read in correct format from the field selector
    const policy = new MonitoringAlertPolicy(this.newPolicy.getValues());
    const specControl = policy.$formGroup.get('spec') as FormGroup;
    // populating destinations
    // Making the values form groups
    let destinationValues = this.destinationsControl.value;
    destinationValues = destinationValues.map((item) => {
      return new FormControl(item);
    });
    specControl.removeControl('destinations');
    specControl.addControl('destinations', new FormArray(destinationValues));


    // Making the values form groups
    let fieldSelectorValues = this.fieldSelector.getValuesWithValueFormAsArray();
    fieldSelectorValues = fieldSelectorValues.map((item) => {
      return new FieldsRequirement(item).$formGroup;
    });
    // To reset a form array, we have to delete the old and readd
    specControl.removeControl('requirements');
    specControl.addControl('requirements', new FormArray(fieldSelectorValues));
    let handler: Observable<{ body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number }>;
    if (this.isInline) {
      handler = this._monitoringService.UpdateAlertPolicy(this.newPolicy.meta.name, policy);
    } else {
      handler = this._monitoringService.AddAlertPolicy(policy);
    }

    handler.subscribe(
      (response) => {
        this.cancelPolicy();
      },
      (error) => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.error('Monitoring service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
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
