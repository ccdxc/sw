import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { HostsComponent } from './hosts.component';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { PrimengModule } from '@lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/api';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { MessageService } from '@app/services/message.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { BehaviorSubject } from 'rxjs';
import { By } from '@angular/platform-browser';
import { RouterLinkStubDirective } from '@common/RouterLinkStub.directive.spec';
import { TestingUtility } from '@common/TestingUtility';
import { ClusterHost } from '@sdk/v1/models/generated/cluster';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { DebugElement } from '@angular/core';
import { AuthService } from '@app/services/auth.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { NewhostComponent } from './newhost/newhost.component';

describe('HostsComponent', () => {
  let component: HostsComponent;
  let fixture: ComponentFixture<HostsComponent>;
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
  const host2 = {
    'kind': 'Host',
    'api-version': 'v1',
    'meta': {
      'name': 'test-host',
      'generation-id': '1',
      'resource-version': '112568',
      'uuid': 'aebeced0-d1a2-4d2f-9d6d-0ca6eff85681',
      'creation-time': '2019-04-03T18:57:31.727876245Z',
      'mod-time': '2019-04-03T18:57:31.727880959Z',
      'self-link': '/configs/cluster/v1/hosts/test-host'
    },
    'spec': {
      'dscs': [
        {
          'id': 'test'
        }
      ]
    },
    'status': {}
  };

  const host3 = {
    'kind': 'Host',
    'api-version': 'v1',
    'meta': {
      'name': 'test-host1',
      'generation-id': '1',
      'resource-version': '228646',
      'uuid': '57779a13-792e-4acb-8a89-93c34be739e4',
      'creation-time': '2019-04-04T20:47:41.054985496Z',
      'mod-time': '2019-04-04T20:47:41.054988288Z',
      'self-link': '/configs/cluster/v1/hosts/test-host1'
    },
    'spec': {
      'dscs': [
        {
          'id': 'test3'
        }
      ]
    },
    'status': {}
  };

  // id test => mac address 00ae.cd00.1142
  const naple1 = {
    'kind': 'DistributedServiceCard',
    'api-version': 'v1',
    'meta': {
      'name': '00ae.cd00.1142',
      'generation-id': '1',
      'resource-version': '706999',
      'uuid': '96fa49f5-ccb8-40ac-a314-41dd798fae78',
      'creation-time': '2019-04-02T18:09:39.17373748Z',
      'mod-time': '2019-04-09T07:15:59.574423516Z',
      'self-link': '/configs/cluster/v1/distributedservicecards/00ae.cd00.1142'
    },
    'spec': {
      'admit': true,
      'id': 'test',
      'ip-config': {
        'ip-address': '1.2.3.4'
      },
      'mgmt-mode': 'NETWORK',
      'network-mode': 'INBAND',
      'controllers': [
        '192.168.30.10'
      ]
    },
    'status': {
      'admission-phase': 'ADMITTED',
      'conditions': [
        {
          'type': 'HEALTHY',
          'status': 'TRUE',
          'last-transition-time': '2019-04-09T07:15:51Z'
        }
      ],
      'serial-num': 'FLM18440006',
      'primary-mac': '00ae.cd00.1142',
      'ip-config': {
        'ip-address': '1.1.1.1'
      },
      'system-info': {
        'bios-info': {
          'version': '1.0E'
        },
        'os-info': {
          'type': 'Linux',
          'kernel-relase': '4.4.0-87-generic',
          'processor': 'ARMv7'
        },
        'cpu-info': {
          'speed': '2.0 Ghz'
        },
        'memory-info': {
          'type': 'HBM'
        }
      },
      'interfaces': [
        'lo',
        'eth0',
        'eth1',
        'eth2'
      ],
      'DSCVersion': '1.0E',
      'smartNicSku': '68-0003-02 01',
      'host': 'test-name5'
    }
  };

  const workload1 = {
    'kind': 'Workload',
    'api-version': 'v1',
    'meta': {
      'name': 'w1',
      'tenant': 'default',
      'namespace': 'default',
      'generation-id': '2',
      'resource-version': '139282',
      'uuid': 'f3b01b40-5f21-4fa2-9c63-e3bb0b243d29',
      'labels': {
        'type': 'test'
      },
      'creation-time': '2019-10-18T20:29:41.577867228Z',
      'mod-time': '2019-10-18T22:55:50.128243229Z',
      'self-link': '/configs/workload/v1/tenant/default/workloads/w1'
    },
    'spec': {
      'host-name': 'test-host',
      'interfaces': [
        {
          'mac-address': 'aaaa.bbbb.cccc',
          'micro-seg-vlan': 1,
          'external-vlan': 1,
          'ip-addresses': [
            '1.1.11.1'
          ]
        }
      ]
    }
  };

  const workload2 = {
    'kind': 'Workload',
    'api-version': 'v1',
    'meta': {
      'name': 'w2',
      'tenant': 'default',
      'namespace': 'default',
      'generation-id': '2',
      'resource-version': '139282',
      'uuid': 'f3b01b40-5f21-4fa2-9c63-e3bb0b243d29',
      'labels': {
        'type': 'test'
      },
      'creation-time': '2019-10-18T20:29:41.577867228Z',
      'mod-time': '2019-10-18T22:55:50.128243229Z',
      'self-link': '/configs/workload/v1/tenant/default/workloads/w1'
    },
    'spec': {
      'host-name': 'naples1-host',
      'interfaces': [
        {
          'mac-address': 'aaaa.bbbb.cccc',
          'micro-seg-vlan': 1,
          'external-vlan': 1,
          'ip-addresses': [
            '1.1.11.1'
          ]
        }
      ]
    }
  };

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [HostsComponent, NewhostComponent, RouterLinkStubDirective],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        PrimengModule,
        WidgetsModule,
        MaterialdesignModule,
        RouterTestingModule,
        SharedModule],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        ClusterService,
        MessageService,
        WorkloadService
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(HostsComponent);
    component = fixture.componentInstance;
  });

  it('should populate table', () => {
    TestingUtility.setAllPermissions();
    const service = TestBed.get(ClusterService);
    spyOn(service, 'WatchHost').and.returnValue(
      TestingUtility.createWatchEventsSubject([
        host1, host2
      ])
    );

    const serviceWorkload = TestBed.get(WorkloadService);
    spyOn(serviceWorkload, 'WatchWorkload').and.returnValue(
      TestingUtility.createWatchEventsSubject([
        workload1, workload2
      ])
    );

    fixture.detectChanges();

    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Hosts (2)');
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('.ui-table-scrollable-body tbody'));
    expect(tableBody).toBeTruthy();

    TestingUtility.verifyTable([new ClusterHost(host2), new ClusterHost(host1)], component.cols, tableBody, {
      'spec.dscs': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        expect(fieldElem.nativeElement.textContent).toContain(
          component.processSmartNics(rowData)[0]['text']
        );  // only works if we for one entry case
      },
      'workloads': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        const workloads = component.getHostWorkloads(rowData);
        expect(workloads.length).toBeGreaterThanOrEqual(0);
        if (workloads.length > 0) {
          expect(fieldElem.nativeElement.textContent).toContain(
            workloads[0].meta.name
          );
        } else {
          expect(fieldElem.nativeElement.textContent.length).toEqual(0);
        }
      }
    }, 'edit delete ', true);
  });

  it('should have correct router links', () => {
    TestingUtility.setAllPermissions();
    const serviceCluster = TestBed.get(ClusterService);
    spyOn(serviceCluster, 'WatchHost').and.returnValue(
      TestingUtility.createWatchEventsSubject([
        host1, host2, host3
      ])
    );

    spyOn(serviceCluster, 'WatchDistributedServiceCard').and.returnValue(
      TestingUtility.createWatchEventsSubject([
        naple1
      ])
    );
    fixture.detectChanges();

    // find DebugElements with an attached RouterLinkStubDirective
    const linkDes = fixture.debugElement
      .queryAll(By.directive(RouterLinkStubDirective));
    // get attached link directive instances
    // using each DebugElement's injector
    const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
    expect(routerLinks.length).toBe(2, 'Should have 2 routerLinks');

    expect(routerLinks[0].linkParams).toBe('/cluster/dscs/00ae.cd00.1142');
    expect(routerLinks[1].linkParams).toBe('/cluster/dscs/0242.c0a8.1c02');
  });

  describe('RBAC', () => {
    beforeEach(() => {
      const service = TestBed.get(ClusterService);
      spyOn(service, 'WatchHost').and.returnValue(
        TestingUtility.createWatchEventsSubject([
          host1, host2, host3
        ])
      );

      spyOn(service, 'WatchDistributedServiceCard').and.returnValue(
        TestingUtility.createWatchEventsSubject([
          naple1
        ])
      );
    });

    it('naples read permission', () => {
      TestingUtility.addPermissions(
        [UIRolePermissions.clusterdistributedservicecard_read]
      );
      fixture.detectChanges();
      const linkDes = fixture.debugElement
        .queryAll(By.directive(RouterLinkStubDirective));
      // get attached link directive instances
      // using each DebugElement's injector
      const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
      expect(routerLinks.length).toBe(2, 'Should have 2 routerLinks');
    });

    it('no permission', () => {
      fixture.detectChanges();
      const linkDes = fixture.debugElement
        .queryAll(By.directive(RouterLinkStubDirective));
      // get attached link directive instances
      // using each DebugElement's injector
      const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
      expect(routerLinks.length).toBe(0, 'Should have no routerLinks');
    });

  });

});
