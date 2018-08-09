import { TestBed, inject } from '@angular/core/testing';

import { UIConfigsService } from './uiconfigs.service';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';

describe('UiconfigsService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [UIConfigsService],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule
      ]
    });
  });

  it('should be created', inject([UIConfigsService], (service: UIConfigsService) => {
    expect(service).toBeTruthy();
  }));
});
