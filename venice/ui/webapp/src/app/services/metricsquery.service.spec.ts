import { TestBed, inject } from '@angular/core/testing';

import { MetricsqueryService } from './metricsquery.service';
import { ControllerService } from './controller.service';
import { LogService } from './logging/log.service';
import { LogPublishersService } from './logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { MessageService } from 'primeng/primeng';

describe('MetricsqueryService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [
        MetricsqueryService,
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule
      ]
    });
  });

  it('should be created', inject([MetricsqueryService], (service: MetricsqueryService) => {
    expect(service).toBeTruthy();
  }));
});
