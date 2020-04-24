import { TestBed, inject, fakeAsync, tick } from '@angular/core/testing';

import { EventsService } from './events.service';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { BehaviorSubject } from 'rxjs';
import { MessageService } from '@app/services/message.service';
import { ApiListWatchOptions_sort_order } from '@sdk/v1/models/generated/events';
import { UIConfigsService } from './uiconfigs.service';
import { AuthService } from './auth.service';
import { LicenseService } from './license.service';

describe('EventsService', () => {
  let postSpy;
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [EventsService,
        ControllerService,
        UIConfigsService,
        LicenseService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ],
      imports: [
        HttpClientTestingModule,
        RouterTestingModule]
    });
    const service = TestBed.get(EventsService);
    service.POLLING_INTERVAL = 10000;
    postSpy = spyOn(service, 'PostGetEvents').and.returnValue(
      new BehaviorSubject({
        body: {
          items: [{ meta: { uuid: 'event1', 'mod-time': '2018-08-20T19:09:04.777255798Z' } }]
        }
      })
    );
  });

  it('polls should be isolated by key', fakeAsync(inject([EventsService], (service: EventsService) => {
    const serviceAny = service as any;
    const handler1 = service.pollEvents('handler1');
    spyOn(handler1, 'next');
    const subscription1 = handler1.subscribe();
    // Using <any> typecast to spy on protected member
    spyOn<any>(service.pollingUtility, 'terminatePolling').and.callThrough();
    tick(5000);
    subscription1.unsubscribe();
    const handler2 = service.pollEvents('handler2');
    const subscription2 = handler2.subscribe((data) => { });
    spyOn(handler2, 'next');
    tick(6000);
    expect(handler1.next).toHaveBeenCalled();
    expect(handler2.next).toHaveBeenCalled();
    // There are no subscribers for handler1 so it should have terminated
    expect(serviceAny.pollingUtility.terminatePolling).toHaveBeenCalledTimes(1);
    tick(5000);
    expect(handler2.next).toHaveBeenCalledTimes(2);

    tick(10000);
    expect(handler2.next).toHaveBeenCalledTimes(3);
    subscription2.unsubscribe();
    tick(10000);
    expect(serviceAny.pollingUtility.terminatePolling).toHaveBeenCalledTimes(2);
  })));

  it('polls with a new body should restart a poll', fakeAsync(inject([EventsService], (service: EventsService) => {
    const serviceAny = service as any;
    // Using <any> typecast to spy on protected member
    spyOn<any>(service.pollingUtility, 'terminatePolling').and.callThrough();
    const pollingSpy = spyOn<any>(service, 'pollingFetchData').and.callThrough();

    const handler1 = service.pollEvents('handler1');
    spyOn(handler1, 'next').and.callThrough();
    const subscription1 = handler1.subscribe();
    tick(1000);
    expect(serviceAny.pollingFetchData).toHaveBeenCalledTimes(1);
    pollingSpy.calls.reset();
    service.pollEvents('handler1', { 'max-results': 100, 'sort-order': ApiListWatchOptions_sort_order.none });
    expect(serviceAny.pollingUtility.terminatePolling).toHaveBeenCalledTimes(1);
    tick(1000);
    expect(serviceAny.pollingFetchData).toHaveBeenCalledTimes(1);
    expect(serviceAny.pollingFetchData).toHaveBeenCalledWith('handler1', { 'max-results': 100, 'field-selector': 'meta.mod-time>2018-08-20T19:09:04.777255798Z', 'sort-order': ApiListWatchOptions_sort_order.none }, true);
    // Shouldn't have appended onto results from the first call
    expect(handler1.next).toHaveBeenCalledWith([{ meta: { uuid: 'event1', 'mod-time': '2018-08-20T19:09:04.777255798Z' } }]);
    subscription1.unsubscribe();
    tick(10000);
    expect(serviceAny.pollingUtility.terminatePolling).toHaveBeenCalledTimes(2);
  })));

  it('polls should automatically change the mod-time of the request', fakeAsync(inject([EventsService], (service: EventsService) => {
    // Poll
    const serviceAny = service as any;
    // Using <any> typecast to spy on protected member
    spyOn<any>(service.pollingUtility, 'terminatePolling').and.callThrough();
    const pollingSpy = spyOn<any>(service, 'pollingFetchData').and.callThrough();

    let handler1 = service.pollEvents('handler1');
    const handlerSpy = spyOn(handler1, 'next').and.callThrough();
    let subscription1 = handler1.subscribe();
    tick(1000);
    expect(serviceAny.pollingFetchData).toHaveBeenCalledTimes(1);
    expect(handler1.next).toHaveBeenCalledTimes(1);
    handlerSpy.calls.reset();
    pollingSpy.calls.reset();
    postSpy.and.returnValue(
      new BehaviorSubject({
        body: {
          items: [{ meta: { uuid: 'event2', 'mod-time': '2018-08-20T19:09:04.777255798Z' } }]
        }
      })
    );
    tick(10000);
    // Check next poll adds mod time
    expect(serviceAny.pollingFetchData).toHaveBeenCalledTimes(1);
    expect(serviceAny.pollingFetchData).toHaveBeenCalledWith('handler1', { 'field-selector': 'meta.mod-time>2018-08-20T19:09:04.777255798Z', 'sort-order': ApiListWatchOptions_sort_order.none }, true);
    expect(handler1.next).toHaveBeenCalledTimes(1);
    // When not blank, should pass in updated array.
    expect(handler1.next).toHaveBeenCalledWith([{ meta: { uuid: 'event2', 'mod-time': '2018-08-20T19:09:04.777255798Z' } }, { meta: { uuid: 'event1', 'mod-time': '2018-08-20T19:09:04.777255798Z' } }]);
    postSpy.and.returnValue(new BehaviorSubject(
      {
        body: {
        }
      }
    ));
    tick(11000);
    // If blank results, next shouldn't be called
    expect(handler1.next).toHaveBeenCalledTimes(1);
    subscription1.unsubscribe();
    tick(10000);
    expect(serviceAny.pollingUtility.terminatePolling).toHaveBeenCalledTimes(1);

    postSpy.and.returnValue(new BehaviorSubject(
      {
        body: {
          items: [{ meta: { uuid: 'event3', 'mod-time': '2018-08-20T19:09:04.777255798Z' } }]
        }
      }
    ));

    // Same scenario but with multiple field selectors
    pollingSpy.calls.reset();
    handler1 = service.pollEvents('handler1', { 'field-selector': 'random>10,meta.mod-time>2018,random2>30', 'sort-order': ApiListWatchOptions_sort_order.none });
    spyOn(handler1, 'next').and.callThrough();
    subscription1 = handler1.subscribe();
    tick(1000);
    expect(serviceAny.pollingFetchData).toHaveBeenCalledTimes(1);
    pollingSpy.calls.reset();
    tick(10000);
    // Check next poll adds mod time
    expect(serviceAny.pollingFetchData).toHaveBeenCalledTimes(1);
    expect(serviceAny.pollingFetchData).toHaveBeenCalledWith('handler1', { 'field-selector': 'random>10,meta.mod-time>2018-08-20T19:09:04.777255798Z,random2>30', 'sort-order': ApiListWatchOptions_sort_order.none }, true);
    expect(handler1.next).toHaveBeenCalledTimes(2);
    // When not blank, should pass in updated array, events deduped
    expect(handler1.next).toHaveBeenCalledWith([{ meta: { uuid: 'event3', 'mod-time': '2018-08-20T19:09:04.777255798Z' } }]);
    postSpy.and.returnValue(new BehaviorSubject(
      {
        body: {
        }
      }
    ));
    tick(10000);
    // If blank results, next shouldn't be called
    expect(handler1.next).toHaveBeenCalledTimes(2);
    subscription1.unsubscribe();
    tick(10000);
    expect(serviceAny.pollingUtility.terminatePolling).toHaveBeenCalledTimes(2);

  })));

  it('polls with bad meta mod-time requests should print an error', fakeAsync(inject([EventsService], (service: EventsService) => {
    const consoleSpy = spyOn(console, 'error');
    const handler1 = service.pollEvents('handler1', { 'field-selector': 'meta.mod-time<10', 'sort-order': ApiListWatchOptions_sort_order.none });
    const subscription = handler1.subscribe();
    tick(1000);
    expect(console.error).toHaveBeenCalledWith('Requests with meta.mod-time<XXX should not be made using polling');
    consoleSpy.calls.reset();
    service.pollEvents('handler1', { 'field-selector': 'meta.mod-time>1,meta.mod-time>2', 'sort-order': ApiListWatchOptions_sort_order.none });
    tick(1000);
    expect(console.error).toHaveBeenCalledWith('Multiple mod-time selectors should be combined into a single one');
    subscription.unsubscribe();
    tick(10000);
  })));

});
