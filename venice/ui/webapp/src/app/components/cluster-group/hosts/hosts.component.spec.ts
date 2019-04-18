import {async, ComponentFixture, fakeAsync, TestBed, tick} from '@angular/core/testing';

import {HostsComponent} from './hosts.component';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {NoopAnimationsModule} from '@angular/platform-browser/animations';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {PrimengModule} from '@lib/primeng.module';
import {WidgetsModule} from 'web-app-framework';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {RouterTestingModule} from '@angular/router/testing';
import {SharedModule} from '@components/shared/shared.module';
import {ControllerService} from '@app/services/controller.service';
import {ConfirmationService} from 'primeng/api';
import {LogService} from '@app/services/logging/log.service';
import {LogPublishersService} from '@app/services/logging/log-publishers.service';
import {MatIconRegistry} from '@angular/material';
import {MessageService} from '@app/services/message.service';
import {ClusterService} from '@app/services/generated/cluster.service';
import {BehaviorSubject} from 'rxjs';
import {By} from '@angular/platform-browser';
import {RouterLinkStubDirective} from '@common/RouterLinkStub.directive.spec';
import {TestingUtility} from '@common/TestingUtility';
import {ClusterHost} from '@sdk/v1/models/generated/cluster';
import {DebugElement} from '@angular/core';

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
      'smart-nics': [
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
      'smart-nics': [
        {
          'name': 'test'
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
      'smart-nics': [
        {
          'name': 'test3'
        }
      ]
    },
    'status': {}
  };

  // hostname test => mac address 00ae.cd00.1142
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
      'hostname': 'test',
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

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [HostsComponent, RouterLinkStubDirective],
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
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        ClusterService,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(HostsComponent);
    component = fixture.componentInstance;
  });

  it('should populate table', () => {
    const service = TestBed.get(ClusterService);
    spyOn(service, 'WatchHost').and.returnValue(
      new BehaviorSubject({
        events: [
          {
            type: 'Created',
            object: host1
          },
          {
            type: 'Created',
            object: host2
          }
        ]
      })
    );

    fixture.detectChanges();

    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Hosts (2)');
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('.ui-table-scrollable-body tbody'));
    expect(tableBody).toBeTruthy();

    TestingUtility.verifyTable([new ClusterHost(host2), new ClusterHost(host1)], component.cols, tableBody, {
      'spec.smart-nics': (fieldElem: DebugElement, rowData: any, rowIndex: number) => {
        expect(fieldElem.nativeElement.textContent).toContain(
          component.processSmartNics(rowData)[0]['text']
        );  // only works if we for one entry case
      }
    });
  });

  it('should have correct router links', () => {
    const service = TestBed.get(ClusterService);
    spyOn(service, 'WatchHost').and.returnValue(
      new BehaviorSubject({
        events: [
          {
            type: 'Created',
            object: host1
          },
          {
            type: 'Created',
            object: host2
          },
          {
            type: 'Created',
            object: host3
          }
        ]
      })
    );

    spyOn(service, 'WatchSmartNIC').and.returnValue(
      new BehaviorSubject({
        events: [
          {
            type: 'Created',
            object: naple1
          }
        ]
      })
    );
    fixture.detectChanges();

    // find DebugElements with an attached RouterLinkStubDirective
    const linkDes = fixture.debugElement
      .queryAll(By.directive(RouterLinkStubDirective));
    // get attached link directive instances
    // using each DebugElement's injector
    const routerLinks = linkDes.map(de => de.injector.get(RouterLinkStubDirective));
    expect(routerLinks.length).toBe(2, 'Should have 2 routerLinks');

    expect(routerLinks[0].linkParams).toBe('/cluster/naples/00ae.cd00.1142');
    expect(routerLinks[1].linkParams).toBe('/cluster/naples/0242.c0a8.1c02');
  });

});
