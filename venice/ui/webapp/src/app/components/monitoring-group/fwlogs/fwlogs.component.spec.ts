/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { ConfirmationService } from 'primeng/primeng';
import { WidgetsModule } from 'web-app-framework';
import { MonitoringGroupModule } from '../monitoring-group.module';
import { FwlogpoliciesComponent } from './fwlogpolicies/fwlogpolicies.component';
import { NewfwlogpolicyComponent } from './fwlogpolicies/newfwlogpolicy/newfwlogpolicy.component';
import { FwlogsComponent } from './fwlogs.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService } from '@app/services/auth.service';
import { TestingUtility } from '@app/common/TestingUtility';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TelemetryqueryService } from '@app/services/generated/telemetryquery.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { Telemetry_queryFwlog, ITelemetry_queryFwlogsQueryResponse, Telemetry_queryFwlog_action, Telemetry_queryFwlog_direction } from '@sdk/v1/models/generated/telemetry_query';
import { By } from '@angular/platform-browser';
import { DebugElement } from '@angular/core';
import { RouterLinkStubDirective } from '@app/common/RouterLinkStub.directive.spec';
import { BehaviorSubject } from 'rxjs';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';


describe('fwlogsComponent', () => {
  let component: FwlogsComponent;
  let fixture: ComponentFixture<FwlogsComponent>;

  const fwlog: ITelemetry_queryFwlogsQueryResponse = { 'tenant': 'default', 'results': [{ 'statement_id': 0, 'logs': [{ 'source': '130.121.45.86', 'destination': '187.173.108.78', 'source-port': 3981, 'destination-port': 137, 'protocol': 'ICMP', 'action': Telemetry_queryFwlog_action.deny, 'direction': Telemetry_queryFwlog_direction.from_uplink, 'rule-id': '3779', 'session-id': '807', 'reporter-id': '00ae.cd00.1142', 'time': '2019-05-14T18:23:31.03798656Z' as any }] }] };

  const naple1 = {
    'kind': 'SmartNIC',
    'api-version': 'v1',
    'meta': {
      'name': '00ae.cd00.1142',
      'generation-id': '1',
      'resource-version': '706999',
      'uuid': '96fa49f5-ccb8-40ac-a314-41dd798fae78',
      'creation-time': '2019-04-02T18:09:39.17373748Z',
      'mod-time': '2019-04-09T07:15:59.574423516Z',
      'self-link': '/configs/cluster/v1/smartnics/00ae.cd00.1142'
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
      'smartNicVersion': '1.0E',
      'smartNicSku': '68-0003-02 01',
      'host': 'test-name5'
    }
  };

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        FwlogsComponent,
        FwlogpoliciesComponent,
        NewfwlogpolicyComponent,
        RouterLinkStubDirective
      ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        WidgetsModule,
        RouterTestingModule,
        MonitoringGroupModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        TelemetryqueryService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MonitoringService,
        MatIconRegistry,
        MessageService,
        ClusterService
      ]
    })
      .compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(FwlogsComponent);
    component = fixture.componentInstance;
  });

  it('should map reporter from mac address to host name', () => {
    TestingUtility.setAllPermissions();
    const clusterService = TestBed.get(ClusterService);
    const telemetryService = TestBed.get(TelemetryqueryService);
    spyOn(telemetryService, 'PostFwlogs').and.returnValue(
      new BehaviorSubject({body: fwlog})
    );
    spyOn(clusterService, 'WatchSmartNIC').and.returnValue(
      TestingUtility.createWatchEvents([
        naple1
      ])
    );
    fixture.detectChanges();
    expect(component).toBeTruthy();
    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Firewall Logs (1)');
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('.ui-table-scrollable-body tbody'));
    expect(tableBody).toBeTruthy();

    TestingUtility.verifyTable(
      fwlog.results[0].logs.map((l) => {
        return new Telemetry_queryFwlog(l);
      }),
      component.cols, tableBody, {
      'reporter-id': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        expect(fieldElem.nativeElement.textContent).toContain(
          naple1.spec.id
        );
      },
      'time': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        expect(fieldElem.nativeElement.textContent).toContain(
         new PrettyDatePipe('en-US').transform(rowData.time, 'ns')
        );
      },
    });

    const linkDes = fixture.debugElement
      .queryAll(By.directive(RouterLinkStubDirective));
    // get attached link directive instances
    // using each DebugElement's injector
    const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
    expect(routerLinks.length).toBe(1, 'Should have 1 routerLinks');
    expect(routerLinks[0].linkParams).toBe('/cluster/naples/00ae.cd00.1142');
  });

  describe('RBAC', () => {
    let toolbarSpy: jasmine.Spy;

    beforeEach(() => {
      TestingUtility.removeAllPermissions();
      const controllerService = TestBed.get(ControllerService);
      toolbarSpy = spyOn(controllerService, 'setToolbarData');
    });

    it('Admin user', () => {
      TestingUtility.setAllPermissions();
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);
    });

    it('fwlogs policy only', () => {
      TestingUtility.addPermissions([UIRolePermissions.monitoringfwlogpolicy_read]);
      fixture.detectChanges();

      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(1);
    });

    it('no access', () => {
      fixture.detectChanges();
      expect(toolbarSpy.calls.mostRecent().args[0].buttons.length).toBe(0);
    });
  });
});

