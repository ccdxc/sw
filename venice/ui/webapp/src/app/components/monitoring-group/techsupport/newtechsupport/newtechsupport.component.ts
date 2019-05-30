import { AfterViewInit, Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, ViewChild, OnDestroy } from '@angular/core';
import { FormArray, AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';
import { Animations } from '@app/animations';
import { Subscription } from 'rxjs';
import { SelectItem } from 'primeng/primeng';

import { Utility } from '@app/common/Utility';
import { ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ClusterSmartNIC } from '@sdk/v1/models/generated/cluster';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterNode } from '@sdk/v1/models/generated/cluster';


import { BaseComponent } from '@app/components/base/base.component';
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

  naples: ReadonlyArray<ClusterSmartNIC> = [];
  naplesEventUtility: HttpEventUtility<ClusterSmartNIC>;


  nodes: ReadonlyArray<ClusterNode> = [];
  nodeEventUtility: HttpEventUtility<ClusterNode>;

  subscriptions: Subscription[] = [];

  nodesOptions: SelectItem[] = [];
  selectedNodes: SelectItem[] = [];
  sourceNaples: SelectItem[] = [];
  targetNaples: SelectItem[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    private clusterService: ClusterService,
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
    this.getNaples();
    this.getNodes();
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
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        this.populateNaples();
      },
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  getNodes() {
    this.nodeEventUtility = new HttpEventUtility<ClusterNode>(ClusterNode);
    this.nodes = this.nodeEventUtility.array;
    const subscription = this.clusterService.WatchNode().subscribe(
      response => {
        this.nodeEventUtility.processEvents(response);
        this.populateNodes();
      },
    );
    this.subscriptions.push(subscription);
  }

  populateNodes () {
    this.nodesOptions.length = 0;
    this.nodes.forEach( (node: ClusterNode) => {
      this.nodesOptions.push({
          label: node.meta.name,
          value: node
        }
      );
    });
  }

  populateNaples() {
    this.sourceNaples.length = 0;
    this.targetNaples.length = 0;
    this.naples.forEach ( (naple: ClusterSmartNIC) => {
      this.sourceNaples.push({
        label: naple.spec.id,  // naple name
        value: naple,
        title: naple.meta.name  // mac address
      });
    });
  }

  /**
   * This API control whether to enable [save tech-support button]
   */
  computeButtonClass() {
    if (Utility.isEmpty(this.newTechsupport.$formGroup.get(['meta', 'name']).value)) {
      return 'global-button-disabled';
    }
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
    if (techsupport.spec['node-selector'].names.length < 1) {
      this._controllerService.invokeErrorToaster('Invalid', 'Please pick node or naples');
      return;
    }
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
    techsupport.spec['node-selector'].names.length = 0;
    this.addNodeNamesHelper(techsupport, this.selectedNodes);
    this.addNodeNamesHelper(techsupport, this.targetNaples);
    return techsupport;
  }

  addNodeNamesHelper(techsupport: IMonitoringTechSupportRequest, selectedNodes: SelectItem[]) {
    const myNodeNames = [];
    selectedNodes.forEach((item: SelectItem) => {
      myNodeNames.push(item.label);
    });
    techsupport.spec['node-selector'].names = techsupport.spec['node-selector'].names.concat(myNodeNames);
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
