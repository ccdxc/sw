import { Component, OnInit, Inject, Output, EventEmitter, ViewEncapsulation } from '@angular/core';
import { MAT_DIALOG_DATA, MatDialogRef, MatDialog } from '@angular/material/dialog';
import { FormGroup, FormArray, FormControl } from '@angular/forms';
import {SelectItem} from 'primeng/primeng';
import { EditableColumn } from 'primeng/table';
import { ClusterCluster, IClusterCluster } from '@sdk/v1/models/generated/cluster';
import { Observable } from 'rxjs';
import { IApiStatus} from '@sdk/v1/models/generated/auth';
import { ControllerService } from '@app/services/controller.service';
import { BaseComponent } from '@app/components/base/base.component';
import { ClusterService } from '@app/services/generated/cluster.service';
import { Utility } from '@app/common/Utility';


@Component({
  selector: 'app-clusterupdate',
  templateUrl: './clusterupdate.component.html',
  styleUrls: ['./clusterupdate.component.scss'],
  encapsulation: ViewEncapsulation.None,
})
export class ClusterupdateComponent extends BaseComponent implements OnInit {

  form: FormGroup;
  @Output() formClose: EventEmitter<any> = new EventEmitter();
  ntp: any = [];
  nodeOptions: SelectItem[] = [];
  tempNTPs: String[] = [];
  editicon: any = {
    name : 'edit',
    margin: {
      top: '0px',
      left: '0px'
    },
  };
  quorumnodes: any = [];
  addNTP: FormControl = new FormControl('');
  tempCluster: ClusterCluster;


  constructor(public dialogRef: MatDialogRef<ClusterupdateComponent>,
    private _clusterService: ClusterService,
    protected _controllerService: ControllerService,
    @Inject(MAT_DIALOG_DATA) public data: any) {
      super(_controllerService);
    }

  ngOnInit() {
    this.tempCluster = new ClusterCluster(this.data.cluster);
    this.form = this.tempCluster.$formGroup;
    this.quorumnodes = this.form.get(['spec', 'quorum-nodes']).value;
    this.ntp = this.form.get(['spec', 'ntp-servers']).value;
    this.getNodeOptions();
  }

  updateQuorumNodes() {
    this.quorumnodes = this.form.get(['spec', 'quorum-nodes']).value;
  }

  processNTPServers() {
    this.tempNTPs.filter(this.checkEmptyNTP);
  }

  checkEmptyNTP(ntp: String) {
    if (ntp == null || ntp === '') {
      return false;
    }
    return true;
  }

  onDeleteQuorum(name: String, index) {
    this.form.get(['spec', 'quorum-nodes']).setValue(this.quorumnodes.slice(0, index).concat(this.quorumnodes.slice(index + 1)));
    this.quorumnodes = this.form.get(['spec', 'quorum-nodes']).value;

  }

  onDeleteNTPServer(server, index) {
    this.form.get(['spec', 'ntp-servers']).setValue(this.ntp.slice(0, index).concat(this.ntp.slice(index + 1)));
    this.ntp = this.form.get(['spec', 'ntp-servers']).value;
  }

  determineAddButtonClass() {
    const serverarray: any = this.form.get(['spec', 'ntp-servers']).value;
    if (this.addNTP.value === null || this.addNTP.value === '') {
      return 'global-button-disabled';
    }
    return '';
  }

  addtoNTPServers() {
    const len = this.ntp.length + 1;
    this.form.get(['spec', 'ntp-servers']).setValue(this.ntp.slice(0, this.ntp.length).concat(len + '.' + this.addNTP.value));
    this.ntp = this.form.get(['spec', 'ntp-servers']).value;
    this.addNTP.reset('');
  }
  getNodeOptions() {
    for (const i of this.data.nodes) {
      const metaform: any = i.meta;
      const lab = metaform.name;
      const val = metaform.name;
      const selectItem: SelectItem = {label : lab, value : val};
      this.nodeOptions.push(selectItem);
    }
  }

  onNoClick(): void {
    this.form.setValue(this.data.cluster);
    this.formClose.emit();
    this.dialogRef.close();
  }

  /** currently the save button is disabled because modifying cluster configuration is not allowed on
   * backend yet. Later, when backend allows cluster fields like ntp-server, quorum-nodes, etc can be modified,
   * we can modify this validateInputs function.
  */
  validateInputs() {
    return 'global-button-disabled';
  }

  saveChanges() {
    const clusterObject: IClusterCluster = this.tempCluster.getFormGroupValues();
    let handler: Observable<{ body: ClusterCluster | IApiStatus | Error, statusCode: number }>;
    handler = this._clusterService.UpdateCluster(clusterObject);
    handler.subscribe(
      (response) => {
        this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, 'Updated Cluster Configuration');
        this.onNoClick();
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
        this.onNoClick();
      }
    );
  }

}
