import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Component, DebugElement } from '@angular/core';
import { ComponentFixture, TestBed, tick, fakeAsync, discardPeriodicTasks, flush } from '@angular/core/testing';
import { MatIconRegistry } from '@angular/material';
import { By } from '@angular/platform-browser';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { TestingUtility } from '@app/common/TestingUtility';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { AuthService } from '@app/services/auth.service';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { SearchService } from '@app/services/generated/search.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { configureTestSuite } from 'ng-bullet';
import { ConfirmationService } from 'primeng/primeng';
import { NaplesComponent } from './naples.component';
import { BehaviorSubject } from 'rxjs';
import { ISearchSearchResponse } from '@sdk/v1/models/generated/search';


@Component({
  template: ''
})
class DummyComponent { }

describe('NaplesComponent', () => {
  let component: NaplesComponent;
  let fixture: ComponentFixture<NaplesComponent>;

  const naples1 = {
    'meta': {
      'name': 'naples1',
      'labels': {
        'Location': 'us-west-A'
      },
      'mod-time': '2018-08-23T17:35:08.534909931Z',
      'creation-time': '2018-08-23T17:30:08.534909931Z'
    },
    'spec': {
      'id': 'naples1-host',
      'admit': true
    },
    'status': {
      'host': 'naples-host-1',
      'ip-config': {
        'ip-address': '0.0.0.0/0'
      },
      'conditions': [
        {'type': 'healthy', 'status': 'false', 'last-transition-time': '2019-12-05T21:36:31Z'},
        {'type': 'reboot_needed', 'status': 'true', 'last-transition-time': '2019-12-05T21:36:31Z'}
      ],
      'primary-mac': '00ae.cd00.1142',
      'admission-phase': 'pending',
      'DSCVersion': '1.0E',
    }
  };

  const naples2 = {
    'meta': {
      'name': 'naples2',
      'labels': {
        'Location': 'us-east-A'
      },
      'mod-time': '2018-08-23T17:25:08.534909931Z',
      'creation-time': '2018-08-23T17:20:08.534909931Z'
    },
    'spec': {
      'id': 'naples2-host',
      'admit': true
    },
    'status': {
      'host': 'naples-host-2',
      'ip-config': {
        'ip-address': '0.0.0.10'
      },
      'conditions': [
        {'type': 'nic_health_unknown', 'status': 'true', 'last-transition-time': '2019-12-05T21:36:31Z'},
        {'type': 'reboot_needed', 'status': 'true', 'last-transition-time': '2019-12-05T21:36:31Z'}
      ],
      'primary-mac': '00ae.cd00.1143',
      'admission-phase': 'admitted',
      'DSCVersion': '1.0E',
    }
  };

  const naples3 = {
    'meta': {
      'name': 'naples3',
      'labels': {
        'Location': 'us-east-A'
      },
      'mod-time': '2018-08-23T17:15:08.534909931Z',
      'creation-time': '2018-08-23T17:20:08.534909931Z'
    },
    'spec': {
      'id': 'naples3-host',
      'admit': true
    },
    'status': {
      'host': 'naples-host-3',
      'ip-config': {
        'ip-address': '0.0.0.10'
      },
      'conditions': [
        {'type': 'healthy', 'status': 'true', 'last-transition-time': '2019-12-05T21:36:31Z'},
        {'type': 'reboot_needed', 'status': 'true', 'last-transition-time': '2019-12-05T21:36:31Z'}
      ],
      'primary-mac': '00ae.cd00.1143',
      'admission-phase': 'rejected',
      'DSCVersion': '1.0E',
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
      declarations: [
        NaplesComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        HttpClientTestingModule,
        NoopAnimationsModule,
        SharedModule,
        MaterialdesignModule,
        PrimengModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        ClusterService,
        MatIconRegistry,
        MetricsqueryService,
        MessageService,
        SearchService,
        WorkloadService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesComponent);
    component = fixture.componentInstance;

    const serviceCluster = TestBed.get(ClusterService);
    const searchService = TestBed.get(SearchService);
    const serviceWorkload = TestBed.get(WorkloadService);

    const searchResp: ISearchSearchResponse = {
      'total-hits': '3'
    };

    spyOn(searchService, 'PostQuery').and.returnValue(
      new BehaviorSubject({
        body: searchResp
      })
    );

    const subject = TestingUtility.createWatchEventsSubject([
      workload1, workload2
    ]);

    spyOn(serviceWorkload, 'ListWorkload').and.returnValue(
      subject
    );

    spyOn(serviceWorkload, 'WatchWorkload').and.returnValue(
      TestingUtility.createWatchEventsSubject([
        workload1, workload2
      ])
    );
    spyOn(serviceCluster, 'WatchDistributedServiceCard').and.returnValue(
      TestingUtility.createWatchEventsSubject([naples1, naples2, naples3])
    );

    subject.complete();
  });

  it('should populate table', <any>fakeAsync(() => {
    TestingUtility.setAllPermissions();
    fixture.detectChanges();
    tick(20000);
    fixture.detectChanges();
    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Distributed Services Cards (3)');
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('.ui-table-scrollable-body tbody'));
    expect(tableBody).toBeTruthy();
    TestingUtility.verifyTable([new ClusterDistributedServiceCard(naples1), new ClusterDistributedServiceCard(naples2), new ClusterDistributedServiceCard(naples3)], component.cols,
    tableBody, {
      'workloads': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        const workloads = component.getDSCWorkloads(rowData);
        expect(workloads.length).toBeGreaterThanOrEqual(0);
        if (workloads.length > 0) {
          expect(fieldElem.nativeElement.textContent).toContain(
            workloads[0].meta.name
          );
        } else {
          expect(fieldElem.nativeElement.textContent.length).toEqual(0);
        }
      },
      'status.conditions': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        expect(rowData.status.conditions.length).toBeGreaterThanOrEqual(0);
        if (rowData.status.conditions.length > 0) {
          expect(fieldElem.nativeElement.textContent).toContain('settings_power');
        }
      }
    }, '', true);
    fixture.destroy();
    discardPeriodicTasks();
    flush();
  }));

  describe('RBAC', () => {
    it('no permission', () => {
    fixture.detectChanges();
      // metrics should be hidden
      const cards = fixture.debugElement.queryAll(By.css('app-herocard'));
      expect(cards.length).toBe(3);
    });

  });

});
