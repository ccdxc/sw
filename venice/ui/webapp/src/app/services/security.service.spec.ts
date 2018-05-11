import { HttpClientTestingModule } from '@angular/common/http/testing';
import { TestBed, inject } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { SecurityService } from './security.service';


describe('SecurityService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [SecurityService],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule]
    });
  });

  it('should be created', inject([SecurityService], (service: SecurityService) => {
    expect(service).toBeTruthy();
  }));
});
