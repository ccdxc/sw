import { inject, TestBed, getTestBed, fakeAsync, tick, discardPeriodicTasks } from '@angular/core/testing';

import { AbstractService } from './abstract.service';
import { HttpTestingController, HttpClientTestingModule } from '@angular/common/http/testing';
import { MockDataUtil } from '@app/common/MockDataUtil';
import { Observable } from 'rxjs/Observable';
import { TestScheduler } from 'rxjs/testing/TestScheduler';

fdescribe('AbstractService', () => {
  let httpMock: HttpTestingController;
  beforeEach(() => {
    TestBed.configureTestingModule({
      imports: [HttpClientTestingModule],
      providers: [AbstractService]
    });
    httpMock = getTestBed().get(HttpTestingController);
  });

  it('should be created', inject([AbstractService], (service: AbstractService) => {
    expect(service).toBeTruthy();
  }));

  it('should handle polling', inject([AbstractService], fakeAsync((service: AbstractService) => {
    // Using <any> typecast to spy on protected member
    const spy1 = spyOn<any>(service, 'pollingHasObservers');
    const spy2 = spyOn<any>(service, 'terminatePolling').and.callThrough();
    const spy3 = spyOn<any>(service, 'pollingFetchData');
    service.initiatePolling(true);
    // There are no observers so it should terminate without fetching data
    tick(5300);
    expect(spy1).toHaveBeenCalledTimes(1);
    expect(spy2).toHaveBeenCalledTimes(1);
    expect(spy3).toHaveBeenCalledTimes(0);
    // Casting to any so that we can check protected member to ensure timer has been destroyed
    expect((service as any).pollingTimerSubscription).toBe(null);

    // Mocking that there are observers
    spy1.and.returnValue(true);
    service.initiatePolling(true);
    tick(5300); // Should be two ticks of fetching data
    expect(spy1).toHaveBeenCalledTimes(3);
    expect(spy2).toHaveBeenCalledTimes(1); // shouldn't have been called again
    expect(spy3).toHaveBeenCalledTimes(2);
    tick(5000);
    expect(spy1).toHaveBeenCalledTimes(4);
    expect(spy2).toHaveBeenCalledTimes(1); // shouldn't have been called again
    expect(spy3).toHaveBeenCalledTimes(3);

    spy1.and.returnValue(false);
    // no more observers, timer should stop
    tick(5000);
    expect(spy1).toHaveBeenCalledTimes(5);
    expect(spy2).toHaveBeenCalledTimes(2);
    expect(spy3).toHaveBeenCalledTimes(3); // shouldn't have been called again since we terminate

    // Casting to any so that we can check protected member to ensure timer has been destroyed
    expect((service as any).pollingTimerSubscription).toBe(null);
  })));
});
