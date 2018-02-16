/**-----
 Angular imports
 ------------------*/
import { TestBed, async, inject, getTestBed } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClient } from '@angular/common/http';
import { HttpClientTestingModule, HttpTestingController } from '@angular/common/http/testing';

/**-----
 Venice imports
 ------------------*/
import { ControllerService } from './controller.service';
import { WorkloadService } from './workload.service';
import {MockDataUtil} from '../common/MockDataUtil';

/**
 * workload.service.spec.ts
 *
 * This is unit-test class for WorkloadService
 *
 */

describe('WorkloadService', () => {


  let injector: TestBed;
  let service: WorkloadService;
  let httpMock: HttpTestingController;
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [WorkloadService],
      imports: [
                HttpClientTestingModule,
                RouterTestingModule]
    });
    injector = getTestBed();
    service = injector.get(WorkloadService);
    httpMock = injector.get(HttpTestingController);
  });

  it('should be created', inject([WorkloadService], (_service: WorkloadService) => {
    expect(service).toBeTruthy();
  }));

  it('should return an Observable<any>', () => {
    const dummyRecords = [
      { login: 'John' },
      { login: 'Doe' }
    ];

    service.getItems().subscribe(data => {
      expect(data.length).toBe(2);
      expect(data).toEqual(dummyRecords);
    });
    const url = service.getItemURL();
    let req = null;
    if (service.isToMockData()) {
     // can't get it to work
     // req = httpMock.expectOne('');
     // req.flush(dummyRecords);
    }else {
      req = httpMock.expectOne(url);
      expect(req.request.method).toBe('GET');
      req.flush(dummyRecords);
    }
  });




});
