import { TestBed, inject } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { VeniceUIHttpInterceptor } from './httpinterceptor.service';

describe('HttpinteceptorService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [VeniceUIHttpInterceptor]
    });
  });

  it('should be created', inject([VeniceUIHttpInterceptor], (service: VeniceUIHttpInterceptor) => {
    expect(service).toBeTruthy();
  }));
});
