import { HttpClientTestingModule } from '@angular/common/http/testing';
import { inject, TestBed } from '@angular/core/testing';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';

import { ControllerService } from './controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from 'primeng/primeng';


describe('ControllerService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule,
      ]
    });
  });

  it('should be created', inject([ControllerService], (service: ControllerService) => {
    expect(service).toBeTruthy();
  }));
});
