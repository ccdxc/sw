import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { PrimengModule } from '@app/lib/primeng.module';
import { MatIconRegistry, MatIcon } from '@angular/material';
import { MatDialog, MatDialogRef, MAT_DIALOG_DATA} from '@angular/material/dialog';
import { MaterialdesignModule} from '@lib/materialdesign.module';
import { FormsModule, ReactiveFormsModule} from '@angular/forms';
import { ClusterupdateComponent } from './clusterupdate.component';
import { SharedModule } from '@app/components/shared/shared.module';
import { ClusterService} from '@app/services/generated/cluster.service';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ControllerService } from '@app/services/controller.service';
import { RouterTestingModule } from '@angular/router/testing';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { AuthService } from '@app/services/auth.service';
import { ConfirmationService } from 'primeng/primeng';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { ClusterNode, ClusterCluster} from '@sdk/v1/models/generated/cluster';
import { NgModule } from '@angular/core';


@NgModule({
  entryComponents: [ClusterupdateComponent]
})
class TestModule { }


describe('ClusterupdateComponent', () => {
  let component: ClusterupdateComponent;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ClusterupdateComponent ],
      imports: [PrimengModule,
        RouterTestingModule,
        ReactiveFormsModule,
        SharedModule,
        FormsModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        TestModule],
      providers: [
        {provide: MatDialogRef, useValue: {}},
        {provide: MAT_DIALOG_DATA, useValue: {}},
        MatIconRegistry,
        MatDialog,
        ControllerService,
        ClusterService,
        LogService,
        LogPublishersService,
        MessageService,
        AuthService,
        ConfirmationService,
        UIConfigsService,
        LicenseService,
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    const dialog = TestBed.get(MatDialog);
    const node1 = new ClusterNode({ 'kind': 'Node', 'api-version': 'v1', 'meta': { 'name': 'node1', 'tenant': 'default', 'namespace': 'default', 'resource-version': '5', 'uuid': '6a000b60-949b-4338-bd7e-8e750a9a8edb', 'creation-time': '2018-06-13T17:50:29.117624431Z', 'mod-time': '2018-06-13T17:50:29.117624431Z', 'self-link': '/v1/cluster/nodes/node1' }, 'spec': {}, 'status': { 'phase': 'joined', 'quorum': true } });
    const componentRef = dialog.open(ClusterupdateComponent, {
      data: {
        nodes : [node1],
        cluster : new ClusterCluster({ 'kind': 'Cluster', 'api-version': 'v1', 'meta': { 'name': 'testCluster', 'tenant': 'default', 'namespace': 'default', 'resource-version': '454', 'uuid': '88f8baa7-7a8e-4a2a-9780-c23d2a217b5d', 'creation-time': '2018-06-13T17:50:29.117538562Z', 'mod-time': '2018-06-13T17:50:44.783819163Z', 'self-link': '/v1/cluster/cluster/testCluster' }, 'spec': { 'quorum-nodes': ['node1', 'node2', 'node3'], 'virtual-ip': '192.168.30.10', 'ntp-servers': ['1.pool.ntp.org', '2.pool.ntp.org'], 'auto-admit-dscs': true }, 'status': { 'leader': 'node1', 'last-leader-transition-time': '2018-06-13T17:50:44.783544086Z' } })
      }
    });
    component = componentRef.componentInstance;
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
