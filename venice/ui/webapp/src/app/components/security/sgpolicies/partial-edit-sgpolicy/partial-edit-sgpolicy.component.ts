import { Component, OnInit, ViewEncapsulation, AfterViewInit, Input, Output, EventEmitter } from '@angular/core';
import { ISecurityNetworkSecurityPolicy, SecurityNetworkSecurityPolicy, SecuritySGRule, SecurityProtoPort, SecurityApp, SecuritySecurityGroup, ISecuritySecurityGroup } from '@sdk/v1/models/generated/security';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { Animations } from '@app/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { OrderedItem } from '@app/components/shared/orderedlist/orderedlist.component';
import { Utility } from '@app/common/Utility';
import { SelectItem } from 'primeng/api';
import { IPUtility } from '@app/common/IPUtility';
import { FormArray, FormControl, ValidatorFn } from '@angular/forms';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { NewsgpolicyComponent } from '@app/components/security/sgpolicies/newsgpolicy/newsgpolicy.component';

@Component({
  selector: 'app-partial-edit-sgpolicy',
  templateUrl: './partial-edit-sgpolicy.component.html',
  styleUrls: ['./partial-edit-sgpolicy.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
})
export class PartialEditSgpolicyComponent extends NewsgpolicyComponent implements OnInit, AfterViewInit {

  @Output() saveEmitter: EventEmitter<any> = new EventEmitter();
  @Output() closeEmitter: EventEmitter<any> = new EventEmitter();

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected securityService: SecurityService,
    protected workloadService: WorkloadService, ) {
    super(_controllerService, uiconfigsService, securityService, workloadService);
  }

  getClassName(): string {
    return this.constructor.name;
  }

  saveObject() {

    const obj = this.getObjectValues();
    this.saveEmitter.emit(obj);
    this.closeEmitter.emit('save');
  }

  cancelObject() {
    this.closeEmitter.emit('close');
  }

}
