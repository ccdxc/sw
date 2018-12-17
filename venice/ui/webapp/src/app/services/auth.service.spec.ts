import { HttpClientTestingModule } from '@angular/common/http/testing';
import { inject, TestBed } from '@angular/core/testing';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';

import { AuthService } from './auth.service';
import { ControllerService } from './controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from 'primeng/primeng';

describe('AuthService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [AuthService,
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

  it('should be created', inject([AuthService], (service: AuthService) => {
    expect(service).toBeTruthy();
  }));
});
