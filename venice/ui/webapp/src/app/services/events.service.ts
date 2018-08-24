import { Injectable } from '@angular/core';
import { EventsService as EventGenService } from '@app/services/generated/events.service';
import { ControllerService } from '@app/services/controller.service';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs/Observable';
import { Subscription } from 'rxjs/Subscription';
import { BehaviorSubject } from 'rxjs/BehaviorSubject';
import { timer } from 'rxjs/observable/timer';
import { IEventsEvent, IEventsEventList, ApiListWatchOptions, IApiListWatchOptions } from '@sdk/v1/models/generated/events';
import { Utility } from '@app/common/Utility';

/**
 * Events polling
 *
 * To allow for multiple components to perform polls with different bodies,
 * each poll request is associated with a key provided by the consumer.
 * Each poll request will automatically be updated with a mod time selector to only
 * fetch new items. If the user makes another call to pollEvents with a different body,
 * the poll will reset and be called with the new body. It will use the same data handler.
 *
 * Note: poll requests with a field-selector with meta.mod-time<XXX isn't supported, as this
 * call should be made as a single request.
 */

interface PollingInstance {
  pollingTimerSource: Observable<number>;
  pollingTimerSubscription: Subscription;
  handler: BehaviorSubject<any>;
  body: any;
  // Used to determine whether the results from the query should
  // be appended to, or if they should wipe the current set
  // Needed for when a consumer makes a request with a new body
  isFirstPoll: boolean;
}

@Injectable()
export class EventsService extends EventGenService {
  POLLING_INTERVAL = 10000;
  events: Array<IEventsEvent> = [];
  protected pollingTimerSource: Observable<number>;
  protected pollingTimerSubscription: Subscription;
  private eventsHandlerMap: { [key: string]: PollingInstance } = {};
  constructor(protected _http: HttpClient,
    protected _controllerService: ControllerService) {
    super(_http, _controllerService);
  }

  pollEvents(key: string, body: IApiListWatchOptions = {}) {
    const poll: PollingInstance = this.initiatePolling(key, body, true, this.POLLING_INTERVAL, 0);
    return poll.handler;
  }

  /**
   * If this returns false, the polling will terminate
   */
  protected pollingHasObservers(key) {
    if (this.eventsHandlerMap[key] == null) {
      return false;
    }
    return this.eventsHandlerMap[key].handler.observers.length !== 0;
  }

  /**
   * Called when the timer ticks to fetch data
   * @param useRealData
   */
  protected pollingFetchData(key, body, useRealData): void {
    const bodyObj = new ApiListWatchOptions(body);
    this.PostGetEvents(bodyObj).subscribe(
      (resp) => {
        const respBody = resp.body as IEventsEventList;
        let items = respBody.items;
        if (items == null) {
          items = [];
        }
        const poll = this.eventsHandlerMap[key];
        if (poll == null) {
          return false;
        }
        if (poll.isFirstPoll) {
          // Don't append data
          this.eventsHandlerMap[key].handler.next(respBody.items);
          poll.isFirstPoll = false;
          // modify body to look for changes after current mod-time
          // Search field-selector for the mod-time attribute, otherwise add it
          if (items.length === 0) {
            // No items, no reason to add time selector
            return;
          }
          // items are sorted by latest time first, so we can look at the first
          this.addModTimeSelector(items[0].meta['mod-time'], poll.body);
        } else {
          // We assume that the incoming poll is only for new data
          if (items.length > 0) {
            const currArray = poll.handler.value;
            const res = items.concat(currArray);
            this.eventsHandlerMap[key].handler.next(res);
            // Modify time selector to only get new data
            this.addModTimeSelector(items[0].meta['mod-time'], poll.body);
          }
        }
      },
      (error) => {
        console.error('Polling for events failed', error);
      }
    );
  }

  /**
   * Adds a mod time selector to only get results after the specified mod time to the
   * request body
   * @param time
   * @param body
   */
  addModTimeSelector(time, body: IApiListWatchOptions) {
    const modSelector = 'meta.mod-time>' + time;
    const fieldSelector = body['field-selector'];
    if (fieldSelector == null || fieldSelector === '') {
      body['field-selector'] = modSelector;
    } else {
      const selectorArray = fieldSelector.split(',');
      // If selector contains one where mod time < XXX, we don't alter the request
      // We could stop the request, that request should be made as a single request, not a poll
      // holds the index for where to replace the time selector
      // Should only be one instance, but there could potentially be more
      const indexes = [];
      selectorArray.forEach((selector, index) => {
        if (selector.indexOf('meta.mod-time') >= 0) {
          if (selector.indexOf('<') >= 0) {
            console.error('Requests with meta.mod-time<XXX should not be made using polling');
          } else {
            // Store the index
            indexes.push(index);
          }
        }
      });
      if (indexes.length > 1) {
        console.error('Multiple mod-time selectors should be combined into a single one');
      }
      if (indexes.length === 0) {
        selectorArray.push(modSelector);
      } else {
        // replace the index
        selectorArray[indexes[0]] = modSelector;
      }
      body['field-selector'] = selectorArray.join(',');
    }
  }

  /**
   * Returns a function to be used for processing the responses of the poll
   * @param key
   * @param useRealData
   */
  genOnPollData(key, useRealData) {
    return () => {
      // Check if there are any subscribers to our subjects
      // if not, we terminate this timer.
      if (!this.pollingHasObservers(key)) {
        this.terminatePolling(key);
        return;
      }
      if (useRealData) {
        // Setting up data to be polled
        const body = this.eventsHandlerMap[key].body;
        this.pollingFetchData(key, body, useRealData);
      }
    };
  }

  /**
   * Initates polling for data
   * Will automatically terminate if pollingHasObservers returns false
   */
  public initiatePolling(key, body, useRealData, interval = 5000, initialDelay = 200): PollingInstance {
    // See if there is already a polling instance existing
    const poll = this.eventsHandlerMap[key];
    if (poll == null || poll.pollingTimerSource == null) {
      const pollingTimerSource = timer(initialDelay, interval);
      const pollingTimerSubscription =
        pollingTimerSource.subscribe(this.genOnPollData(key, useRealData));

      this.eventsHandlerMap[key] = {
        pollingTimerSource: pollingTimerSource,
        pollingTimerSubscription: pollingTimerSubscription,
        handler: new BehaviorSubject<IEventsEvent[]>([]),
        body: body,
        isFirstPoll: true
      };
      return this.eventsHandlerMap[key];
    }
    // If there is a polling instance, we check if the body is the same
    // as the one in the current request. If it isn't, we restart the poll
    // with the new body.
    const bodyPoll = poll.body;
    if (!Utility.getLodash().isEqual(body, bodyPoll)) {
      // stop current poll and start again with new body.
      this.terminatePolling(key, false);
      const pollingTimerSource = timer(initialDelay, interval);
      const pollingTimerSubscription =
        pollingTimerSource.subscribe(this.genOnPollData(key, useRealData));
      poll.pollingTimerSource = pollingTimerSource;
      poll.pollingTimerSubscription = pollingTimerSubscription;
      poll.body = body;
    }
    return this.eventsHandlerMap[key];
  }

  /**
   * terminates the polling timer
   */
  protected terminatePolling(key, terminateHandler = true) {
    const poll = this.eventsHandlerMap[key];
    if (poll != null && poll.pollingTimerSubscription != null) {
      poll.pollingTimerSubscription.unsubscribe();
      poll.pollingTimerSource = null;
      poll.pollingTimerSubscription = null;
      poll.body = null;
      poll.isFirstPoll = true;
      if (terminateHandler) {
        poll.handler.complete();
        poll.handler = null;
      }
    }
  }



}
