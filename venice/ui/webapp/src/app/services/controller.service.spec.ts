
import { TestBed, async, inject } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClient } from '@angular/common/http';
import { HttpClientTestingModule } from '@angular/common/http/testing';

import { ControllerService } from './controller.service';

describe('ControllerService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ControllerService],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule
    ]
    });
  });

  it('should be created', inject([ControllerService], (service: ControllerService) => {
    expect(service).toBeTruthy();
  }));
});
