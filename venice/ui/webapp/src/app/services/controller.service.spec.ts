import { HttpClientTestingModule } from '@angular/common/http/testing';
import { inject, TestBed } from '@angular/core/testing';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';

import { ControllerService } from './controller.service';


describe('ControllerService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ControllerService, MatIconRegistry],
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
