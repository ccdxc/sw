import { AfterViewInit, Component, Input, OnInit, ViewEncapsulation, ViewChild, OnDestroy } from '@angular/core';
import { FormArray, ValidatorFn } from '@angular/forms';
import { Animations } from '@app/animations';
import { SelectItem } from 'primeng/primeng';

import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterNode } from '@sdk/v1/models/generated/cluster';


import { IMonitoringTechSupportRequest, MonitoringTechSupportRequest } from '@sdk/v1/models/generated/monitoring';
import { RepeaterComponent, RepeaterData, ValueType } from 'web-app-framework';
import { FieldsRequirement_operator } from '@sdk/v1/models/generated/monitoring';
import { CreationForm } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';


@Component({
  selector: 'app-newtechsupport',
  templateUrl: './newtechsupport.component.html',
  styleUrls: ['./newtechsupport.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NewtechsupportComponent extends CreationForm<IMonitoringTechSupportRequest, MonitoringTechSupportRequest> implements OnInit, AfterViewInit, OnDestroy {
  @ViewChild('cslabelRepeater') cslabelRepeater: RepeaterComponent;
  @ViewChild('nmlabelRepeater') nmlabelRepeater: RepeaterComponent;

  @Input() existingTechSupportRequest: MonitoringTechSupportRequest[] = [];

  cslabelData: RepeaterData[] = [];
  nmlabelData: RepeaterData[] = [];
  cslabelFormArray = new FormArray([]);
  nmlabelFormArray = new FormArray([]);

  naples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  naplesEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;


  nodes: ReadonlyArray<ClusterNode> = [];
  nodeEventUtility: HttpEventUtility<ClusterNode>;

  nodesOptions: SelectItem[] = [];
  selectedNodes: SelectItem[] = [];
  sourceNaples: SelectItem[] = [];
  targetNaples: SelectItem[] = [];

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    protected uiconfigsService: UIConfigsService,
    private clusterService: ClusterService,
  ) {
    super(_controllerService, uiconfigsService, MonitoringTechSupportRequest);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit() {
    this.newObject.$formGroup.get(['meta', 'name']).setValidators([
      this.newObject.$formGroup.get(['meta', 'name']).validator,
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

  setToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary techsupportrequests-toolbar-button techsupportrequests-toolbar-SAVE',
        text: 'CREATE TECH SUPPORT REQUEST',
        callback: () => { this.saveObject(); },
        computeClass: () => this.computeButtonClass()
      },
      {
        cssClass: 'global-button-neutral techsupportrequests-toolbar-button techsupportrequests-toolbar-CANCEL',
        text: 'CANCEL',
        callback: () => { this.cancelObject(); }
      },
    ];

    this._controllerService.setToolbarData(currToolbar);
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterDistributedServiceCard>(ClusterDistributedServiceCard);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterDistributedServiceCard>;
    const subscription = this.clusterService.WatchDistributedServiceCard().subscribe(
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
      this._controllerService.webSocketErrorHandler('Failed to get Nodes')
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
    this.naples.forEach ( (naple: ClusterDistributedServiceCard) => {
      this.sourceNaples.push({
        label: naple.spec.id,  // naple name
        value: naple,
        title: naple.meta.name  // mac address
      });
    });
  }

  isFormValid(): boolean {
    if (Utility.isEmpty(this.newObject.$formGroup.get(['meta', 'name']).value)) {
      return false;
    }
    if (!this.newObject.$formGroup.valid) {
      return false;
    }
    // we want user picks nodes or naples
    if (this.selectedNodes.length === 0 && this.targetNaples.length  === 0) {
      return false;
    }
    return true;
  }

  isTechSupportRequestNameValid(existingTechSupportRequest: MonitoringTechSupportRequest[]): ValidatorFn {
    return Utility.isModelNameUniqueValidator(existingTechSupportRequest, 'tech-support-request-name');
  }

  getObjectValues(): IMonitoringTechSupportRequest {
    const techsupport: IMonitoringTechSupportRequest = this.newObject.getFormGroupValues();
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

  createObject(techsupport: IMonitoringTechSupportRequest) {
    if (techsupport.spec['node-selector'].names.length < 1) {
      this._controllerService.invokeErrorToaster('Invalid', 'Please pick node or naples');
      return null;
    }
    return this._monitoringService.AddTechSupportRequest(techsupport);
  }

  updateObject(newObject: IMonitoringTechSupportRequest, oldObject: IMonitoringTechSupportRequest) {
    // unimplemented
    return null;
  }

  generateCreateSuccessMsg(object: IMonitoringTechSupportRequest) {
    return 'Created tech support request ' + object.meta.name;
  }

  generateUpdateSuccessMsg(object: IMonitoringTechSupportRequest) {
    return 'Updated tech support request ' + object.meta.name;
  }

}
