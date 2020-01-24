import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
import { Component } from '@angular/core';

import { HttpClientTestingModule } from '@angular/common/http/testing';

import { GridsterModule } from 'angular-gridster2';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '../../lib/materialdesign.module';

import { FormsModule } from '@angular/forms';
import { ControllerService } from '../../services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { SharedModule } from '@components/shared/shared.module';

import { DashboardComponent } from './dashboard.component';
import { SoftwareversionComponent } from './widgets/softwareversion/softwareversion.component';
import { SystemcapacitywidgetComponent } from './widgets/systemcapacity/systemcapacity.component';
import { NaplesComponent } from './widgets/naples/naples.component';
import { PolicyhealthComponent } from './widgets/policyhealth/policyhealth.component';
import { DsbdworkloadComponent } from './widgets/dsbdworkload/dsbdworkload.component';

import { MatIconRegistry } from '@angular/material';
import { ConfirmationService } from 'primeng/primeng';
import { WorkloadsComponent } from './workloads/workloads.component';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { AuthService } from '@app/services/auth.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { By } from '@angular/platform-browser';
import { TestingUtility } from '@app/common/TestingUtility';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  template: ''
})
class DummyComponent { }

describe('DashboardComponent', () => {
  let component: DashboardComponent;
  let fixture: ComponentFixture<DashboardComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [
        DashboardComponent,
        SystemcapacitywidgetComponent,
        SoftwareversionComponent,
        NaplesComponent,
        PolicyhealthComponent,
        DsbdworkloadComponent,
        DummyComponent,
        WorkloadsComponent,
      ],
      imports: [

        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        HttpClientTestingModule,
        GridsterModule,
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
        NoopAnimationsModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        AuthService,
        AuthServiceGen,
        ConfirmationService,
        MatIconRegistry,
        LogService,
        LogPublishersService,
        MessageService,
        MetricsqueryService,
        ClusterService,
        WorkloadService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(DashboardComponent);
    component = fixture.componentInstance;
    component.gridsterOptions = [];
  });

  describe('RBAC', () => {

    beforeEach(() => {
      TestingUtility.removeAllPermissions();
    });

    it('no permission', () => {
      fixture.detectChanges();
      const cards = fixture.debugElement.queryAll(By.css('app-flip'));
      expect(cards.length).toBe(0);
      TestingUtility.removeAllPermissions();
    });

    it('cluster card', () => {
      TestingUtility.addPermissions([UIRolePermissions.clustercluster_read]);
      fixture.detectChanges();
      // metrics should be hidden
      let cards = fixture.debugElement.queryAll(By.css('app-flip'));
      expect(cards.length).toBe(0);

      TestingUtility.addPermissions([UIRolePermissions.clustercluster_read, UIRolePermissions.clusternode_read]);
      fixture.detectChanges();
      // metrics should be hidden
      cards = fixture.debugElement.queryAll(By.css('app-flip'));
      expect(cards.length).toBe(1);
    });

    it('naples card', () => {
      TestingUtility.addPermissions([UIRolePermissions.clusterdistributedservicecard_read]);
      fixture.detectChanges();
      // metrics should be hidden
      const cards = fixture.debugElement.queryAll(By.css('app-flip'));
      expect(cards.length).toBe(1);
    });

    it('workload card', () => {
      TestingUtility.addPermissions([UIRolePermissions.workloadworkload_read, UIRolePermissions.clusterhost_read]);
      fixture.detectChanges();
      // metrics should be hidden
      const cards = fixture.debugElement.queryAll(By.css('app-flip'));
      expect(cards.length).toBe(1);
    });

    it('policy health card', () => {
      TestingUtility.addPermissions([UIRolePermissions.metrics_read]);
      fixture.detectChanges();
      // metrics should be hidden
      const cards = fixture.debugElement.queryAll(By.css('app-flip'));
      expect(cards.length).toBe(1);
    });

  });
});
