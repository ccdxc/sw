import { ChangeDetectionStrategy, Component, EventEmitter, Inject, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { AbstractControl, FormControl, FormGroup, ValidationErrors, ValidatorFn } from '@angular/forms';
import { MatDialogRef, MAT_DIALOG_DATA } from '@angular/material/dialog';
import { IPUtility } from '@app/common/IPUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { IApiStatus } from '@sdk/v1/models/generated/auth';
import { ClusterCluster, IClusterCluster } from '@sdk/v1/models/generated/cluster';
import { required } from '@sdk/v1/utils/validators';
import * as _ from 'lodash';
import { SelectItem } from 'primeng/primeng';
import { Observable } from 'rxjs';


@Component({
  selector: 'app-clusterupdate',
  templateUrl: './clusterupdate.component.html',
  styleUrls: ['./clusterupdate.component.scss'],
  encapsulation: ViewEncapsulation.None,
  changeDetection: ChangeDetectionStrategy.OnPush
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

    // Set validators
    this.setValidators();
  }

  setValidators() {
    this.addNTP.setValidators([this.isNTPServerNameValid()]);
    this.tempCluster.$formGroup.get(['spec', 'virtual-ip']).setValidators([IPUtility.isValidIPValidator, required]);
  }

  isNTPServerNameValid(): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const ntpValues = this.form.get(['spec', 'ntp-servers']).value;
      const isNameExist = ( ntpValues.findIndex( val =>  val === control.value)) >= 0;
      if (isNameExist) {
        return {
          objectname: {
            required: true,
            message: 'NTP Name is required and must be unique.'
          }
        };
      }
      return null;
    };
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
    if (! this.isNTPServerInputValid()) {
      return 'global-button-disabled';
    }
    return '';
  }

  isNTPServerInputValid(): boolean {
    if ( this.addNTP.dirty &&  this.addNTP.errors ) {
      return false;
    } else if (!this.addNTP.dirty || Utility.isEmpty( this.addNTP.value )) {
      return false;
    }
    return true;
  }

  addtoNTPServers() {
    this.form.get(['spec', 'ntp-servers']).setValue(this.ntp.slice(0, this.ntp.length).concat(this.addNTP.value));
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
   // this.form.setValue(this.data.cluster);
   //  this.tempCluster.setFormGroupValuesToBeModelValues();
    this.formClose.emit();
    this.dialogRef.close();
  }

  /**
   * Control whether to enable "commit change" button
   *
   */
  validateInputs() {
    return ( this.isFormValid()) ? '' : 'global-button-disabled';
  }

  /**
    * If form is not dirty, consider form is not valid
   * otherwise, check form.validity
   */
  isFormValid(): boolean {
    /* if (!this.tempCluster.$formGroup.dirty) {
      return false;
    } */
    if (! this.tempCluster.$formGroup.valid) {
      return false;
    }
    return this.isFormValueChanged();
  }

  private isFormValueChanged() {
    const modelValue = Utility.trimUIFields(this.tempCluster.getModelValues());
    const formValue = Utility.trimUIFields(this.tempCluster.getFormGroupValues());
    return !_.isEqual(formValue, modelValue);
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
