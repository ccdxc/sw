import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaplesComponent } from './naples.component';
import { Component } from '@angular/core';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MatIconRegistry } from '@angular/material';
import { BehaviorSubject } from 'rxjs/BehaviorSubject';
import { By } from '@angular/platform-browser';
import { TestingUtility } from '@app/common/TestingUtility';
import { ClusterSmartNIC } from '@sdk/v1/models/generated/cluster';
import { MessageService } from 'primeng/primeng';

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
      'ip-config': {
        'ip-address': '0.0.0.0/0'
      },
      'hostname': 'naples1-host'
    },
    'status': {
      'admission-phase': 'ADMITTED',
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
      'ip-config': {
        'ip-address': '0.0.0.10'
      },
      'hostname': 'naples2-host'
    },
    'status': {
      'admission-phase': 'ADMITTED',
    }
  };

  beforeEach(async(() => {
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
        LogService,
        LogPublishersService,
        ClusterService,
        MatIconRegistry,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesComponent);
    component = fixture.componentInstance;
  });

  it('should populate table', () => {
    const service = TestBed.get(ClusterService);
    spyOn(service, 'WatchSmartNIC').and.returnValue(
      new BehaviorSubject({
        body: {
          result: {
            Events: [
              {
                Type: 'Created',
                Object: naples1
              },
              {
                Type: 'Created',
                Object: naples2
              }
            ]
          }
        }
      })
    );
    fixture.detectChanges();
    // check table header
    const title = fixture.debugElement.query(By.css('.tableheader-title'));
    expect(title.nativeElement.textContent).toContain('Naples (2)');
    // check table contents
    const tableBody = fixture.debugElement.query(By.css('tbody'));
    expect(tableBody).toBeTruthy();
    TestingUtility.verifyTable([new ClusterSmartNIC(naples2), new ClusterSmartNIC(naples1)], component.cols, tableBody);
  });
});
