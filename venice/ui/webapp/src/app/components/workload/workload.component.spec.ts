/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed, inject } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { FormsModule } from '@angular/forms';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
/**-----
 VeniceUI Framework -  imports
 ------------------*/
import { WidgetsModule } from 'web-app-framework';

/**-----
 Venice UI -  imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
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
import { BehaviorSubject } from 'rxjs/BehaviorSubject';
import { By } from '@angular/platform-browser';
import { TestingUtility } from '@app/common/TestingUtility';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';

@Component({
  template: ''
})
class DummyComponent { }

describe('WorkloadComponent', () => {
  let component: WorkloadComponent;
  let fixture: ComponentFixture<WorkloadComponent>;

  const workload1 = new WorkloadWorkload({
    "meta": {
      "name": "workload1",
      "labels": {
        "Location": "us-west-A"
      },
      "mod-time": '2018-08-23T17:35:08.534909931Z',
      "creation-time": '2018-08-23T17:30:08.534909931Z'
    },
    "spec": {
      "host-name": "esx-host1.local",
      "interfaces": {
        "00:50:56:00:00:03": {
          "micro-seg-vlan": 103,
          "external-vlan": 1003
        },
        "00:50:56:00:00:04": {
          "micro-seg-vlan": 103,
          "external-vlan": 1003
        }
      }
    },
    "status": {
      "interfaces": {
        "00:50:56:00:00:03": {
          "ip-addresses": [
            "10.1.1.1, 11.1.1.1"
          ]
        }
      }
    }
  });

  const workload2 = new WorkloadWorkload({
    "meta": {
      "name": "workload2",
      "labels": {
        "Location": "us-west-A"
      },
      "mod-time": '2018-08-23T17:35:08.534909931Z',
      "creation-time": '2018-08-23T17:30:08.534909931Z'
    },
    "spec": {
      "host-name": "esx-host1.local",
      "interfaces": {
        "00:50:56:00:00:05": {
          "micro-seg-vlan": 104,
          "external-vlan": 1004
        },
        "00:50:56:00:00:06": {
          "micro-seg-vlan": 104,
          "external-vlan": 1004
        }
      }
    },
    "status": {
      "interfaces": {
        "00:50:56:00:00:05": {
          "ip-addresses": [
            "10.1.1.1"
          ]
        },
        "00:50:56:00:00:06": {
          "ip-addresses": [
            "10.1.1.1", "11.1.1.1"
          ]
        }
      }
    }
  });

  const workload3 = new WorkloadWorkload({
    "meta": {
      "name": "workload3",
      "mod-time": '2018-08-23T17:35:08.534909931Z',
      "creation-time": '2018-08-23T17:30:08.534909931Z'
    },
    "spec": {
      "host-name": "esx-host1.local",
      "interfaces": {
        "00:50:56:00:00:05": {
          "micro-seg-vlan": 104,
          "external-vlan": 1004
        },
        "00:50:56:00:00:06": {
          "micro-seg-vlan": 104,
          "external-vlan": 1004
        }
      }
    },
    "status": {
      "interfaces": {
        "00:50:56:00:00:05": {
        },
        "00:50:56:00:00:06": {
        }
      }
    }
  });


  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [WorkloadComponent,
        WorkloadwidgetComponent,
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
        BrowserAnimationsModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        WorkloadServiceGen,
        WorkloadService,
        MatIconRegistry,
        UIConfigsService
      ]
    })
      .compileComponents();

  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(WorkloadComponent);
    component = fixture.componentInstance;
  });

  it('should populate table', () => {
    const service = TestBed.get(WorkloadServiceGen);
    spyOn(service, 'WatchWorkload').and.returnValue(
      new BehaviorSubject({
        body: {
          result: {
            Events: [
              {
                Type: "Created",
                Object: workload1.getValues()
              },
              {
                Type: "Created",
                Object: workload2.getValues()
              },
              {
                Type: "Created",
                Object: workload3.getValues()
              }
            ]
          }
        }
      })
    );
    fixture.detectChanges();
    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Workloads (3)')
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('tbody'));
    expect(tableBody).toBeTruthy();

    const caseMap = {
      'spec.interfaces': (field, rowData, rowIndex) => {
        const macs = Object.keys(rowData.spec.interfaces);
        macs.forEach((mac) => {
          expect(field.nativeElement.textContent)
            .toContain(mac, 'interface column did not contain ' + mac + ' for row ' + rowIndex);
          const ips = rowData.status.interfaces[mac]
          if (ips != null && ips["ip-addresses"] != null) {
            ips["ip-addresses"].forEach((ip) => {
              expect(field.nativeElement.textContent)
                .toContain(ip, 'interface column did not contain ' + mac + ' for row ' + rowIndex);
            });
          }
        })
      }
    }
    TestingUtility.verifyTable([workload1, workload2, workload3], component.cols, tableBody, caseMap);
  });

});
