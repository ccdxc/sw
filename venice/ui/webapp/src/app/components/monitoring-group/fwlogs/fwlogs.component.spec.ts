/**-----
 Angular imports
 ------------------*/
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed, fakeAsync, tick, discardPeriodicTasks, flush } from '@angular/core/testing';
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
import { SecurityService } from '@app/services/generated/security.service';
import { SecurityNetworkSecurityPolicy } from '@sdk/v1/models/generated/security';


describe('fwlogsComponent', () => {
  let component: FwlogsComponent;
  let fixture: ComponentFixture<FwlogsComponent>;
  let securityService;
  let sgPolicyObserver;

  const fwlog: ITelemetry_queryFwlogsQueryResponse = { 'tenant': 'default', 'results': [{ 'statement_id': 0, 'logs': [{ 'source': '130.121.45.86', 'destination': '187.173.108.78', 'source-port': 3981, 'destination-port': 137, 'protocol': 'ICMP', 'action': Telemetry_queryFwlog_action.deny, 'direction': Telemetry_queryFwlog_direction['from-uplink'], 'rule-id': '3779', 'session-id': '807', 'session-state': 'create', 'reporter-id': '00ae.cd00.1142', 'time': '2019-05-14T18:23:31.03798656Z' as any }] }] };

  const policy1 = new SecurityNetworkSecurityPolicy({
    meta: {
      name: 'policy1',
      'mod-time': '2018-08-23T17:35:08.534909931Z',
      'creation-time': '2018-08-23T17:30:08.534909931Z'
    },
    spec: {
      rules: [ {} ]
    },
    status: {
      'rule-status': [ {'rule-hash': '3779'} ]
    }

  });

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
        ClusterService,
        SecurityService
      ]
    })
      .compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(FwlogsComponent);
    component = fixture.componentInstance;
    securityService = TestBed.get(SecurityService);
  });

  it('check if sgicon onhover details are displayed correctly', fakeAsync(() => {
    TestingUtility.setAllPermissions();

    sgPolicyObserver = new BehaviorSubject({
      events: [
        {
          type: 'Created',
          object: policy1
        }
      ]
    });
    spyOn(securityService, 'WatchNetworkSecurityPolicy').and.returnValue(
      sgPolicyObserver
    );
    const telemetryService = TestBed.get(TelemetryqueryService);
    spyOn(telemetryService, 'PostFwlogs').and.returnValue(
      new BehaviorSubject({body: fwlog})
    );

    fixture.detectChanges();
    tick(1000);
    fixture.detectChanges();
    expect(component).toBeTruthy();

    const row = fixture.debugElement.query(By.css('tr'));
    row.triggerEventHandler('mouseenter', null);
    fixture.detectChanges();

    const icon = fixture.debugElement.query(By.css('mat-icon.global-table-action-svg-icon'));
    icon.triggerEventHandler('mouseenter', icon);
    fixture.detectChanges();

    const overlay = fixture.debugElement.query(By.css('.fwlogs-ruletext'));
    const rulehashdiv = overlay.children[1].childNodes[2];
    expect(rulehashdiv.nativeNode.textContent).toBe(fwlog.results[0].logs[0]['rule-id']);
    fixture.destroy();
    discardPeriodicTasks();
    flush();
  }));

 it('should map reporter from mac address to host name', fakeAsync(() => {
    TestingUtility.setAllPermissions();
    const clusterService = TestBed.get(ClusterService);
    const telemetryService = TestBed.get(TelemetryqueryService);
    spyOn(telemetryService, 'PostFwlogs').and.returnValue(
      new BehaviorSubject({body: fwlog})
    );
    spyOn(clusterService, 'WatchDistributedServiceCard').and.returnValue(
      TestingUtility.createWatchEventsSubject([
        naple1
      ])
    );
    sgPolicyObserver = new BehaviorSubject({
      events: [
        {
          type: 'Created',
          object: policy1
        }
      ]
    });
    spyOn(securityService, 'WatchNetworkSecurityPolicy').and.returnValue(
      sgPolicyObserver
    );
    fixture.detectChanges();
    tick(1000);
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
      'policy': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        expect(fieldElem.nativeElement.textContent).toContain(
          policy1.meta.name
        );
      },
    });

    const linkDes = fixture.debugElement
      .queryAll(By.directive(RouterLinkStubDirective));
    // get attached link directive instances
    // using each DebugElement's injector
    const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
    expect(routerLinks.length).toBe(2, 'Should have 2 routerLinks');
    expect(routerLinks[0].linkParams).toBe('/cluster/dscs/00ae.cd00.1142');
    fixture.destroy();
    discardPeriodicTasks();
    flush();
  }));

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

    it('sgpolicy info should not be available to user without permission', () => {

      TestingUtility.addPermissions([UIRolePermissions['fwlogsquery_read']]);

      sgPolicyObserver = new BehaviorSubject({
        events: [
          {
            type: 'Created',
            object: policy1
          }
        ]
      });

      spyOn(securityService, 'WatchNetworkSecurityPolicy').and.returnValue(
        sgPolicyObserver
      );
      const telemetryService = TestBed.get(TelemetryqueryService);
      spyOn(telemetryService, 'PostFwlogs').and.returnValue(
        new BehaviorSubject({body: fwlog})
      );

      fixture.detectChanges();
      expect(component).toBeTruthy();

      const row = fixture.debugElement.query(By.css('tr'));
      row.triggerEventHandler('mouseenter', null);
      fixture.detectChanges();

      const icon = fixture.debugElement.query(By.css('mat-icon.global-table-action-svg-icon'));
      expect(icon).toBeFalsy();

      const headers = fixture.debugElement.queryAll(By.css('th'));
      expect(headers.length).toBe(component.cols.length - 1);

    });
  });
});

