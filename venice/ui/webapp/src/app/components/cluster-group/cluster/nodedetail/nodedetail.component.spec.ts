import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { NodedetailComponent } from './nodedetail.component';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { MessageService } from '@app/services/message.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ActivatedRoute } from '@angular/router';
import { BehaviorSubject } from 'rxjs';
import { RouterLinkStubDirective } from '@app/common/RouterLinkStub.directive.spec';
import { RouterTestingModule } from '@angular/router/testing';
import { AlerttableService } from '@app/services/alerttable.service';
import { SearchService } from '@app/services/generated/search.service';
import { EventsService } from '@app/services/events.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { AuthService } from '@app/services/auth.service';
import { TestingUtility } from '@app/common/TestingUtility';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { By } from '@angular/platform-browser';

class MockActivatedRoute extends ActivatedRoute {
  id = 'node1';
  paramObserver = new BehaviorSubject<any>({ id: this.id });
  snapshot: any = { url: ['security', 'sgpolicies', 'policy1'] };

  constructor() {
    super();
    this.params = this.paramObserver.asObservable();
  }

  setPolicyId(id) {
    this.id = id;
    this.paramObserver.next({ id: this.id });
  }
}

describe('NodedetailComponent', () => {
  let component: NodedetailComponent;
  let fixture: ComponentFixture<NodedetailComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [NodedetailComponent, RouterLinkStubDirective],
      imports: [
        RouterTestingModule,
        HttpClientTestingModule,
        NoopAnimationsModule,
        SharedModule,
        MaterialdesignModule,
        PrimengModule,
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        MessageService,
        ClusterService,
        AlerttableService,
        MetricsqueryService,
        SearchService,
        EventsService,
        AuthService,
        MonitoringService,
        {
          provide: ActivatedRoute,
          useClass: MockActivatedRoute
        },
        MessageService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(NodedetailComponent);
    component = fixture.componentInstance;
  });

  it('should create', () => {
    fixture.detectChanges();
    expect(component).toBeTruthy();
  });

  describe('RBAC', () => {
    it('no permission', () => {
    fixture.detectChanges();
      // metrics should be visible
      const cards = fixture.debugElement.queryAll(By.css('app-herocard'));
      expect(cards.length).toBe(3);
    });

  });
});
