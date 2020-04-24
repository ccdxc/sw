import { TestBed, inject, ComponentFixture, fakeAsync, tick } from '@angular/core/testing';

import { MetricsqueryService, TelemetryPollingMetricQueries } from './metricsquery.service';
import { ControllerService } from './controller.service';
import { LogService } from './logging/log.service';
import { LogPublishersService } from './logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { ConfirmationService } from 'primeng/primeng';
import { MessageService } from './message.service';
import { Component } from '@angular/core';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { throwError, BehaviorSubject, Subscription } from 'rxjs';
import { UIConfigsService } from './uiconfigs.service';
import { AuthService } from './auth.service';
import { LicenseService } from './license.service';

@Component({
  template: ''
})
class DummyComponent {
  metricStatus: string =  'init';
  metricSub: Subscription;
  metricsqueryService;

  constructor(metricsqueryService: MetricsqueryService) {
    this.metricsqueryService = metricsqueryService;
  }

  startQuery() {
    const queryList: TelemetryPollingMetricQueries = {
      queries: [ MetricsUtility.timeSeriesQueryPolling('DistributedServiceCard', [])],
      tenant: 'default'
    };
    this.metricSub = this.metricsqueryService.pollMetrics('test', queryList).subscribe(
      (data) => {
        this.metricStatus = 'ready';
      },
      (err) => {
        this.metricStatus = 'failed';
      }
    );
  }

  stopQuery() {
    this.metricSub.unsubscribe();
  }
}

describe('MetricsqueryService', () => {
  let component: DummyComponent;
  let fixture: ComponentFixture<DummyComponent>;
  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [
        DummyComponent
      ],
      providers: [
        MetricsqueryService,
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule
      ]
    }).compileComponents();
  });

  it('should be created', inject([MetricsqueryService], fakeAsync(inject([MetricsqueryService], (service: MetricsqueryService) => {
    expect(service).toBeTruthy();
    fixture = TestBed.createComponent(DummyComponent);
    component = fixture.componentInstance;
    const spy = spyOn(service, 'PostMetrics').and.returnValue(new BehaviorSubject({
      results: []
    }));
    component.startQuery();
    expect(component.metricStatus = 'ready');

    spy.and.returnValue(throwError({statusCode: 401}));
    tick();
    expect(component.metricStatus = 'failed');
    spy.and.returnValue(new BehaviorSubject({
      results: []
    }));
    tick(200);

    component.stopQuery();
    expect(component.metricStatus).toBe('failed');
    component.startQuery();
    expect(component.metricStatus).toBe('ready');

    component.stopQuery();
    tick();

  }))));
});
