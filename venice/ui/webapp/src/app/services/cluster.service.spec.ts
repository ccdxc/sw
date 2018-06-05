import { TestBed, inject, getTestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule, HttpTestingController } from '@angular/common/http/testing';

/**-----
 Venice imports
 ------------------*/
 import { ControllerService } from './controller.service';
 import { ClusterService } from './cluster.service';

describe('ClusterService', () => {

  let injector: TestBed;
  let service: ClusterService;
  let httpMock: HttpTestingController;

  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [ClusterService],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule
      ]
    });
    injector = getTestBed();
    service = injector.get(ClusterService);
    httpMock = injector.get(HttpTestingController);
  });

  it('should be created', inject([ClusterService], (service: ClusterService) => {
    expect(service).toBeTruthy();
  }));
});
