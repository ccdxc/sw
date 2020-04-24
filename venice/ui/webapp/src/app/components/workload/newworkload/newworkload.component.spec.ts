import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { TestingUtility } from '@app/common/TestingUtility';
import { MonitoringGroupModule } from '@app/components/monitoring-group/monitoring-group.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { AuthService } from '@app/services/auth.service';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { IWorkloadWorkload, WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import * as _ from 'lodash';
import { configureTestSuite } from 'ng-bullet';
import { ConfirmationService } from 'primeng/primeng';
import { NewworkloadComponent } from './newworkload.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { TrimUIFields } from '@sdk/v1/utils/utility';


describe('NewworkloadComponent', () => {
  let component: NewworkloadComponent;
  let fixture: ComponentFixture<NewworkloadComponent>;
  let tu: TestingUtility;
  const host1 = {
    'kind': 'Host',
    'api-version': 'v1',
    'meta': {
      'name': 'naples1-host',
      'generation-id': '1',
      'resource-version': '694',
      'uuid': '0fd7d80e-ba31-411d-a4a8-df2a47bf8cf8',
      'creation-time': '2019-04-02T18:09:37.972814339Z',
      'mod-time': '2019-04-02T18:09:37.972817316Z',
      'self-link': '/configs/cluster/v1/hosts/naples1-host'
    },
    'spec': {
      'dscs': [
        {
          'mac-address': '0242.c0a8.1c02'
        }
      ]
    },
    'status': {}
  };

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [NewworkloadComponent],
      imports: [
        MonitoringGroupModule,
        SharedModule,
        ReactiveFormsModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FormsModule,
        NoopAnimationsModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        WorkloadService,
        MessageService,
        ClusterService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(NewworkloadComponent);
    tu = new TestingUtility(fixture);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    const workloadService = TestBed.get(WorkloadService);
    const spy = spyOn(workloadService, 'AddWorkload');
    // newWorkload has @Input variables. We build them here
    const hosts = [];
    hosts.push(host1);
    component.existingObjects = hosts;
    component.hostOptions = hosts.map( x => {
      return { label: x.meta.name, value: x.meta.name };
    });
    fixture.detectChanges();

    const workload: IWorkloadWorkload = {
      meta: {
        name: 'workload1'
      },
      spec: {
        'host-name': 'naples1-host',
        interfaces: [{
          'mac-address': '1111.1111.1111',
          'micro-seg-vlan': 2,
          'external-vlan': 2,
        }],
      }
    };

    tu.setInput('.new-workload-name', workload.meta.name);
    tu.setDropdown('.new-workload-host', 'naples1-host');
    tu.setInput('.new-workload-mac-address', workload.spec.interfaces[0]['mac-address']);
    tu.setInput('.new-workload-micro-seg-vlan', workload.spec.interfaces[0]['micro-seg-vlan']);
    tu.setInput('.new-workload-external-vlan', workload.spec.interfaces[0]['external-vlan']);

    component.saveObject();
    fixture.detectChanges();
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[0];
    const expVal = TrimUIFields(new WorkloadWorkload(workload).getModelValues());
    expect(_.isEqual(recVal, expVal)).toBeTruthy('Received: ' + JSON.stringify(recVal) + ' , expected: ' + JSON.stringify(expVal));
  });

  it('should update', () => {
    component.isInline = true;
    const workload: IWorkloadWorkload = {
      meta: {
        name: 'workload1'
      },
      spec: {
        'host-name': 'naples1-host',
        interfaces: [{
          'mac-address': '1111.1111.1111',
          'micro-seg-vlan': 2,
          'external-vlan': 2,
        }],
      }
    };
    component.objectData = workload;
    const workloadService = TestBed.get(WorkloadService);
    const clusterService = TestBed.get(ClusterService);
    const spy = spyOn(workloadService, 'UpdateWorkload');
    spyOn(clusterService, 'WatchHost').and.returnValue(
      TestingUtility.createWatchEventsSubject([
        host1,
      ])
    );
    fixture.detectChanges();
    workload.spec.interfaces[0]['mac-address'] = '2222.2222.2222';
    tu.sendClick(tu.getElemByCss('.new-workload-save'));
    expect(spy).toHaveBeenCalled();
    const recVal = spy.calls.mostRecent().args[1];
    const expVal = TrimUIFields(new WorkloadWorkload(workload).getModelValues());
    expect(_.isEqual(recVal.meta, expVal.meta)).toBeTruthy('Received: ' + JSON.stringify(recVal.meta) + ' , expected: ' + JSON.stringify(expVal.meta));
  });
});
