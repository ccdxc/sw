/**-----
 Angular imports
 ------------------*/
import {  ComponentFixture, TestBed, inject, fakeAsync, tick, discardPeriodicTasks, flush } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
/**-----
 VeniceUI Framework -  imports
 ------------------*/
import { WidgetsModule } from 'web-app-framework';

/**-----
 Venice UI -  imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { WorkloadService as WorkloadServiceGen } from '@app/services/generated/workload.service';
import { WorkloadService } from '@app/services/workload.service';

import { WorkloadComponent } from './workload.component';
import { WorkloadwidgetComponent } from './workloadwidget/workloadwidget.component';
import { SharedModule } from '@app/components/shared//shared.module';

/**-----
 Third-parties imports
 ------------------*/
import { MomentModule } from 'angular2-moment';
import { MatIconRegistry } from '@angular/material';


import { PrimengModule } from '@lib/primeng.module';
import { Component } from '@angular/core';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { BehaviorSubject } from 'rxjs';
import { By } from '@angular/platform-browser';
import { TestingUtility } from '@app/common/TestingUtility';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { MessageService } from '@app/services/message.service';
import { AuthService } from '@app/services/auth.service';
import { NewworkloadComponent } from './newworkload/newworkload.component';
import { ClusterService } from '@app/services/generated/cluster.service';
import { SecurityService } from '@app/services/generated/security.service';
import { SearchService } from '@app/services/generated/search.service';
import { ISearchSearchResponse } from '@sdk/v1/models/generated/search';


@Component({
  template: ''
})
class DummyComponent { }

describe('WorkloadComponent', () => {
  let component: WorkloadComponent;
  let fixture: ComponentFixture<WorkloadComponent>;

  const workload1 = new WorkloadWorkload({
    'meta': {
      'name': 'workload1',
      'labels': {
        'Location': 'us-west-A'
      },
      'mod-time': '2018-08-23T17:35:08.534909931Z',
      'creation-time': '2018-08-23T17:30:08.534909931Z'
    },
    'spec': {
      'host-name': 'esx-host1.local',
      'interfaces': {
        '0050.5600.0003': {
          'micro-seg-vlan': 103,
          'external-vlan': 1003
        },
        '0050.5600.0004': {
          'micro-seg-vlan': 103,
          'external-vlan': 1003
        }
      }
    },
    'status': {
      'interfaces': {
        '0050.5600.0003': {
          'ip-addresses': [
            '10.1.1.1, 11.1.1.1'
          ]
        }
      }
    }
  });

  const workload2 = new WorkloadWorkload({
    'meta': {
      'name': 'workload2',
      'labels': {
        'Location': 'us-west-A'
      },
      'mod-time': '2018-08-23T17:35:08.534909931Z',
      'creation-time': '2018-08-23T17:30:08.534909931Z'
    },
    'spec': {
      'host-name': 'esx-host1.local',
      'interfaces': {
        '0050.5600.0005': {
          'micro-seg-vlan': 104,
          'external-vlan': 1004
        },
        '0050.5600.0006': {
          'micro-seg-vlan': 104,
          'external-vlan': 1004
        }
      }
    },
    'status': {
      'interfaces': {
        '0050.5600.0005': {
          'ip-addresses': [
            '10.1.1.1'
          ]
        },
        '0050.5600.0006': {
          'ip-addresses': [
            '10.1.1.1', '11.1.1.1'
          ]
        }
      }
    }
  });

  const workload3 = new WorkloadWorkload({
    'meta': {
      'name': 'workload3',
      'mod-time': '2018-08-23T17:35:08.534909931Z',
      'creation-time': '2018-08-23T17:30:08.534909931Z'
    },
    'spec': {
      'host-name': 'esx-host1.local',
      'interfaces': {
        '0050.5600.0005': {
          'micro-seg-vlan': 104,
          'external-vlan': 1004
        },
        '0050.5600.0006': {
          'micro-seg-vlan': 104,
          'external-vlan': 1004
        }
      }
    },
    'status': {
      'interfaces': {
        '0050.5600.0005': {
        },
        '0050.5600.0006': {
        }
      }
    }
  });


  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [WorkloadComponent,
        WorkloadwidgetComponent,
        NewworkloadComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        HttpClientTestingModule,
        // VeniceUI Framework
        WidgetsModule,
        // Moment.js
        MomentModule,
        // primeNG.js
        PrimengModule,
        MaterialdesignModule,
        SharedModule,
        ReactiveFormsModule,
        FormsModule,
        BrowserAnimationsModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        WorkloadServiceGen,
        WorkloadService,
        MatIconRegistry,
        UIConfigsService,
        MessageService,
        AuthService,
        ClusterService,
        SecurityService,
        SearchService
      ]
    });

  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WorkloadComponent);
    component = fixture.componentInstance;
  });

  it('should populate table', fakeAsync(() => {
    TestingUtility.setAllPermissions();
    const service = TestBed.get(WorkloadServiceGen);
    const searchService = TestBed.get(SearchService);

    const searchResp: ISearchSearchResponse = {
      'total-hits': '3'
    };

    spyOn(searchService, 'PostQuery').and.returnValue(
      new BehaviorSubject({
        body: searchResp
      })
    );
    spyOn(service, 'WatchWorkload').and.returnValue(
      new BehaviorSubject({
        events: [
          {
            type: 'Created',
            object: workload1.getFormGroupValues()
          },
          {
            type: 'Created',
            object: workload2.getFormGroupValues()
          },
          {
            type: 'Created',
            object: workload3.getFormGroupValues()
          }
        ]
      })
    );

    fixture.detectChanges();
    tick(1000);
    fixture.detectChanges();
    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Workloads (3)');
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('.ui-table-scrollable-body tbody'));
    expect(tableBody).toBeTruthy();

    const caseMap = {
      'spec.interfaces': (field, rowData, rowIndex) => {
        const macs = Object.keys(rowData.spec.interfaces);
        macs.forEach((mac) => {
          expect(field.nativeElement.textContent)
            .toContain(mac, 'interface column did not contain ' + mac + ' for row ' + rowIndex);
          const ips = rowData.status.interfaces[mac];
          if (ips != null && ips['ip-addresses'] != null) {
            ips['ip-addresses'].forEach((ip) => {
              expect(field.nativeElement.textContent)
                .toContain(ip, 'interface column did not contain ' + mac + ' for row ' + rowIndex);
            });
          }
        });
      }
    };
    TestingUtility.verifyTable([workload3, workload2, workload1], component.cols, tableBody, caseMap, ' editdelete', true);
    fixture.destroy();
    discardPeriodicTasks();
    flush();
  }));

});
