import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NodedetailComponent } from './nodedetail.component';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from 'primeng/primeng';
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

  beforeEach(async(() => {
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
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        MessageService,
        ClusterService,
        AlerttableService,
        MetricsqueryService,
        SearchService,
        EventsService,
        MonitoringService,
        {
          provide: ActivatedRoute,
          useClass: MockActivatedRoute
        },
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NodedetailComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
