import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';

import { SystemcapacitywidgetComponent } from './systemcapacity.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MatIconRegistry } from '@angular/material';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { MessageService } from '@app/services/message.service';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { AuthService } from '@app/services/auth.service';
import { ClusterNode, ClusterNodeCondition_status } from '@sdk/v1/models/generated/cluster';
import { TestingUtility } from '@app/common/TestingUtility';
import {BehaviorSubject} from 'rxjs';


describe('SystemcapacitywidgetComponent', () => {
  let component: SystemcapacitywidgetComponent;
  let fixture: ComponentFixture<SystemcapacitywidgetComponent>;


  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [SystemcapacitywidgetComponent],
      imports: [
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        NoopAnimationsModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        ClusterService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        MessageService,
        AuthService,
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SystemcapacitywidgetComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});

describe('node cluster test', () => {
  let component: SystemcapacitywidgetComponent;
  let fixture: ComponentFixture<SystemcapacitywidgetComponent>;
  let watchSubject: BehaviorSubject<any>;
  const node1 = new ClusterNode({
    'kind' : 'Node',
    'meta' : {
      'name' : 'node1'
    },
    'status' : {
      'conditions' : [{
        'status' : 'true',
        'type' : 'healthy'
      }],
      'phase' : 'joined',
    }
  });
  const node2 = new ClusterNode({
    'kind' : 'Node',
    'meta' : {
      'name' : 'node2'
    },
    'status' : {
      'conditions' : [{
        'status' : 'false',
        'type' : 'healthy'
      }],
      'phase' : 'joined',
    }
  });
  const node3 = new ClusterNode({
    'kind' : 'Node',
    'meta' : {
      'name' : 'node3'
    },
    'status' : {
      'conditions' : [{
        'status' : 'false',
        'type' : 'healthy'
      }],
      'phase' : 'joined',
    }
  });
  const node4 = new ClusterNode({
    'kind' : 'Node',
    'meta' : {
      'name' : 'node4'
    },
    'status' : {
      'conditions' : [{
        'status' : 'unknown',
        'type' : 'healthy'
      }],
      'phase' : 'joined',
    }
  });

  const node5 = new ClusterNode({
    'kind' : 'Node',
    'meta' : {
      'name' : 'node5'
    },
    'status' : {
      'conditions' : [{
        'status' : 'unknown',
        'type' : 'healthy'
      }],
      'phase' : 'joined',
    }
  });


  configureTestSuite(() => {
    TestBed.configureTestingModule({
     declarations: [SystemcapacitywidgetComponent],
     imports: [
       WidgetsModule,
       PrimengModule,
       MaterialdesignModule,
       RouterTestingModule,
       HttpClientTestingModule,
       NoopAnimationsModule,
       SharedModule
     ],
     providers: [
       ControllerService,
       ConfirmationService,
       LogService,
       LogPublishersService,
       ClusterService,
       MatIconRegistry,
       UIConfigsService,
       MessageService,
       AuthService,
       LicenseService
     ]
   });
     });

  beforeEach(() => {
    fixture = TestBed.createComponent(SystemcapacitywidgetComponent);
    component = fixture.componentInstance;
    const service = TestBed.get(ClusterService);
    watchSubject = TestingUtility.createWatchEventsSubject([node1, node2, node3, node4]) ;
    spyOn(service, 'WatchNode').and.returnValue(
      watchSubject
    );

  });

  it('number of healthy, unhealthy and unknown nodes should be updated correctly on changes', () => {
    TestingUtility.setAllPermissions();
    fixture.detectChanges();
    // check if condition of nodes is initially set correctly and the numbers of each condition is correct
    expect(component.unknownnodes).toBe(1);
    expect(component.unhealthynodes).toBe(2);
    expect(component.healthynodes).toBe(1);
    node3.status.conditions[0].status = ClusterNodeCondition_status.true;
    watchSubject.next({
      events: [
        {
          type : 'Updated',
          object: node3
        }
      ]
    });
    // check if the number of nodes of each condition updates  appropriately when the node(s) condition changes
    expect(component.healthynodes).toBe(2);
    expect(component.unhealthynodes).toBe(1);
    expect(component.unknownnodes).toBe(1); // should remain unaffected
    watchSubject.next({
      events: [
        {
        type : 'Created',
        object: node5
        }
      ]
    });
    // check if number of nodes updates appropriately if a node(s) is added
    expect(component.unknownnodes).toBe(2);
    expect(component.healthynodes).toBe(2);
    expect(component.unhealthynodes).toBe(1);
    watchSubject.next({
      events: [
        {
        type : 'Deleted',
        object: node2
        }
      ]
    });
    // check if number of nodes updates appropriately if a node(s) is deleted
    expect(component.unknownnodes).toBe(2);
    expect(component.healthynodes).toBe(2);
    expect(component.unhealthynodes).toBe(0);
  });
});
