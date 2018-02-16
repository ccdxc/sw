import { TestBed, inject, getTestBed} from '@angular/core/testing';
import { HttpClientTestingModule, HttpTestingController } from '@angular/common/http/testing';

import { AbstractService } from './abstract.service';

describe('AbstractService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [AbstractService]
    });
  });

  it('should be created', inject([AbstractService], (service: AbstractService) => {
    expect(service).toBeTruthy();
  }));
});
