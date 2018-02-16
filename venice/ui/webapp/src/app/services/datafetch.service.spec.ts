import { TestBed, async, inject } from '@angular/core/testing';
import { getTestBed} from '@angular/core/testing';
import { HttpClientTestingModule, HttpTestingController } from '@angular/common/http/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClient } from '@angular/common/http';

import { DatafetchService } from './datafetch.service';

/**
 * Override the read DatafetchService
 */
class UnitTestDatafetchService extends DatafetchService {
  public isToMockData(): boolean {
    // It does not invoke mock-data code
    return false;
  }
}
/**
 *  This is DatafetchService unit test.
 *  It contains general patterns of writing unit-test for http-service (httpClient)
 *
 */
describe('DatafetchService', () => {
  let injector: TestBed;
  let service: UnitTestDatafetchService;
  let httpMock: HttpTestingController;
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [DatafetchService, UnitTestDatafetchService],
      imports: [
                HttpClientTestingModule,
                RouterTestingModule]
    });
    injector = getTestBed();
    service = injector.get(UnitTestDatafetchService);
    httpMock = injector.get(HttpTestingController);
  });

  it('should be created', inject([DatafetchService], (_service: DatafetchService) => {
    expect(service).toBeTruthy();
  }));

  it('should return an Observable<any>', () => {
    const dummyRecords = [
      { login: 'John' },
      { login: 'Doe' }
    ];

    service.globalSearch('dummy').subscribe(data => {
      expect(data.length).toBe(2);
      expect(data).toEqual(dummyRecords);
    });
    const url = service.getGlobalSearchURL();
    const req = httpMock.expectOne(url);
    expect(req.request.method).toBe('GET');
    req.flush(dummyRecords);
  });
});
