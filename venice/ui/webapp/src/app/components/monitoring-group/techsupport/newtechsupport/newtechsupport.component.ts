import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild, OnDestroy } from '@angular/core';
import { FormArray, AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { BaseComponent } from '@app/components/base/base.component';
import { required } from '@sdk/v1/utils/validators';
import { IMonitoringTechSupportRequest, MonitoringTechSupportRequest } from '@sdk/v1/models/generated/monitoring';
import { RepeaterComponent, RepeaterData, ValueType } from 'web-app-framework';
import { FieldsRequirement_operator } from '@sdk/v1/models/generated/monitoring';


@Component({
  selector: 'app-newtechsupport',
  templateUrl: './newtechsupport.component.html',
  styleUrls: ['./newtechsupport.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewtechsupportComponent extends BaseComponent implements OnInit, AfterViewInit, OnDestroy {
  @ViewChild('cslabelRepeater') cslabelRepeater: RepeaterComponent;
  @ViewChild('nmlabelRepeater') nmlabelRepeater: RepeaterComponent;

  oldButtons: ToolbarButton[] = [];
  @Input() existingTechSupportRequest: MonitoringTechSupportRequest[] = [];
  @Output() formClose: EventEmitter<any> = new EventEmitter();
  newTechsupport: MonitoringTechSupportRequest;

  cslabelData: RepeaterData[] = [];
  nmlabelData: RepeaterData[] = [];
  cslabelFormArray = new FormArray([]);
  nmlabelFormArray = new FormArray([]);

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this.newTechsupport = new MonitoringTechSupportRequest();
    this.newTechsupport.$formGroup.get(['meta', 'name']).setValidators([
      this.newTechsupport.$formGroup.get(['meta', 'name']).validator,
      this.isTechSupportRequestNameValid(this.existingTechSupportRequest)]);

    this.cslabelData = [
      {
        key: { label: 'text', value: 'text' },
        operators: Utility.convertEnumToSelectItem(FieldsRequirement_operator),
        fieldType: ValueType.inputField,
        valueType: ValueType.inputField
      }
    ];

    this.nmlabelData = [
      {
        key: { label: 'text', value: 'text' },
        operators: Utility.convertEnumToSelectItem(FieldsRequirement_operator),
        fieldType: ValueType.inputField,
        valueType: ValueType.inputField
      }
    ];
  }

  ngAfterViewInit() {

    const currToolbar = this._controllerService.getToolbarData();
    this.oldButtons = currToolbar.buttons;
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary techsupportrequests-toolbar-button techsupportrequests-toolbar-SAVE',
        text: 'CREATE TECH SUPPORT REQUEST',
        callback: () => { this.onSaveTechsupportRequest(); },
        computeClass: () => this.computeButtonClass()
      },
      {
        cssClass: 'global-button-neutral techsupportrequests-toolbar-button techsupportrequests-toolbar-CANCEL',
        text: 'CANCEL',
        callback: () => { this.onCancelTechsupportRequest(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  ngOnDestroy() {
  }

  /**
   * This API control whether to enable [save tech-support button]
   */
  computeButtonClass() {
    if (this.newTechsupport.$formGroup.valid) {
      return '';
    } else {
      return 'global-button-disabled';
    }
  }

  isTechSupportRequestNameValid(existingTechSupportRequest: MonitoringTechSupportRequest[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingTechSupportRequest, 'tech-support-request-name');
  }

  saveTechsupportRequest() {
    const techsupport: IMonitoringTechSupportRequest = this.buildTechSupportRequest();
    this._monitoringService.AddTechSupportRequest(techsupport).subscribe(
      (response) => {
        this._controllerService.invokeSuccessToaster('Success', 'Tech Support ' + techsupport.meta.name);
        this.setPreviousToolbar(); // Once successfully save tech-support, re-store previous toolbar buttons.
        this.formClose.emit();
      },
      this._controllerService.restErrorHandler('Fail to add tech support:' + techsupport.meta.name)
    );
  }

  buildTechSupportRequest(): IMonitoringTechSupportRequest {
    const techsupport: IMonitoringTechSupportRequest = this.newTechsupport.getFormGroupValues();

   /* Comment it out as backend-does not support label-selector yet
    techsupport.spec['collection-selector'].requirements = Utility.convertRepeaterValuesToSearchExpression(this.cslabelRepeater);
    techsupport.spec['node-selector'].labels.requirements = Utility.convertRepeaterValuesToSearchExpression(this.nmlabelRepeater);
   */
    if (!Array.isArray(techsupport.spec['node-selector'].names)) {
      const nsNames: string = techsupport.spec['node-selector'].names;
      const myNames = [];
      nsNames.split(',').forEach(nsName => {
        myNames.push(nsName.trim());
      });
      techsupport.spec['node-selector'].names = myNames;
    }
    return techsupport;
  }

  onSaveTechsupportRequest() {
    this.saveTechsupportRequest();
  }

  onCancelTechsupportRequest() {

    // Need to reset the toolbar that we changed
    this.setPreviousToolbar();
    this.newTechsupport.$formGroup.reset();
    this.formClose.emit();
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
}
