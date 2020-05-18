import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { ActivatedRoute } from '@angular/router';
import { RouterTestingModule } from '@angular/router/testing';
import { configureTestSuite } from 'ng-bullet';
import { ConfirmationService } from 'primeng/primeng';
import { BehaviorSubject } from 'rxjs';
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { AlerttableService } from '@app/services/alerttable.service';
import { AuthService } from '@app/services/auth.service';
import { ControllerService } from '@app/services/controller.service';
import { EventsService } from '@app/services/events.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { BrowserService } from '@app/services/generated/browser.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { SearchService } from '@app/services/generated/search.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { LicenseService } from '@app/services/license.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { NetworkService } from '@app/services/generated/network.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { WorkloaddetailComponent } from './workloaddetail.component';

class MockActivatedRoute extends ActivatedRoute {
  id = 'sample-name';
  paramObserver = new BehaviorSubject<any>({ id: this.id });
  snapshot: any = { url: ['workload', 'sample-name'], queryParamMap: { get: () => { } } };

  constructor() {
    super();
    this.params = this.paramObserver.asObservable();
  }

  setId(id) {
    this.id = id;
    this.paramObserver.next({ id: this.id });
  }
}

describe('WorkloaddetailComponent', () => {
  let component: WorkloaddetailComponent;
  let fixture: ComponentFixture<WorkloaddetailComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        WorkloaddetailComponent
      ],
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
        AuthServiceGen,
        MonitoringService,
        BrowserService,
        WorkloadService,
        {
          provide: ActivatedRoute,
          useClass: MockActivatedRoute
        },
        NetworkService
      ]
    }).compileComponents();
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(WorkloaddetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should be created', () => {
    expect(component).toBeTruthy();
  });
});
