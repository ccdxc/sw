import { Observable } from 'rxjs/Observable';
import { Subscription } from 'rxjs/Subscription';
import { BehaviorSubject } from 'rxjs/BehaviorSubject';
import { timer } from 'rxjs/observable/timer';
import { Utility } from '@app/common/Utility';

/**
 * To allow for multiple components to perform polls with different bodies,
 * each poll request is associated with a key provided by the consumer.
 * Each poll request will automatically be updated with a mod time selector to only
 * fetch new items. If the user makes another call to pollEvents with a different body,
 * the poll will reset and be called with the new body. It will use the same data handler.
 */

export interface PollingInstance {
  pollingTimerSource: Observable<number>;
  pollingTimerSubscription: Subscription;
  handler: BehaviorSubject<any>;
  body: any;
  // Used to determine whether the results from the query should
  // be appended to, or if they should wipe the current set
  // Needed for when a consumer makes a request with a new body
  isFirstPoll: boolean;
}

export class PollUtility {
  POLLING_INTERVAL = 10000;
  pollingFetchData: (key, body, useRealData) => void;
  protected pollingTimerSource: Observable<number>;
  protected pollingTimerSubscription: Subscription;
  public pollingHandlerMap: { [key: string]: PollingInstance } = {};

  constructor(
    pollingFetchData: (key, body, useRealData) => void,

    pollingInterval: number) {
    this.POLLING_INTERVAL = pollingInterval;
    this.pollingFetchData = pollingFetchData;
  }

  /**
   * If this returns false, the polling will terminate
   */
  protected pollingHasObservers(key) {
    if (this.pollingHandlerMap[key] == null) {
      return false;
    }
    return this.pollingHandlerMap[key].handler.observers.length !== 0;
  }

  /**
   * Returns a function to be used for processing the responses of the timer poll
   * @param key
   * @param useRealData
   */
  genOnPollTimer(key, useRealData) {
    return () => {
      // Check if there are any subscribers to our subjects
      // if not, we terminate this timer.
      if (!this.pollingHasObservers(key)) {
        this.terminatePolling(key);
        return;
      }
      if (useRealData) {
        // Setting up data to be polled
        const body = this.pollingHandlerMap[key].body;
        this.pollingFetchData(key, body, useRealData);
      }
    };
  }

  /**
   * Initates polling for data
   * Will automatically terminate if pollingHasObservers returns false
   */
  public initiatePolling(key, body, useRealData, interval = 5000, initialDelay = 200, defaultValue: any = []): PollingInstance {
    // See if there is already a polling instance existing
    const poll = this.pollingHandlerMap[key];
    if (poll == null || poll.pollingTimerSource == null) {
      const pollingTimerSource = timer(initialDelay, interval);
      const pollingTimerSubscription =
        pollingTimerSource.subscribe(this.genOnPollTimer(key, useRealData));

      this.pollingHandlerMap[key] = {
        pollingTimerSource: pollingTimerSource,
        pollingTimerSubscription: pollingTimerSubscription,
        handler: new BehaviorSubject<any>(defaultValue),
        body: body,
        isFirstPoll: true
      };
      return this.pollingHandlerMap[key];
    }
    // If there is a polling instance, we check if the body is the same
    // as the one in the current request. If it isn't, we restart the poll
    // with the new body.
    const bodyPoll = poll.body;
    if (!Utility.getLodash().isEqual(body, bodyPoll)) {
      // stop current poll and start again with new body.
      poll.handler.next([]);
      this.terminatePolling(key, false);
      const pollingTimerSource = timer(initialDelay, interval);
      const pollingTimerSubscription =
        pollingTimerSource.subscribe(this.genOnPollTimer(key, useRealData));
      poll.pollingTimerSource = pollingTimerSource;
      poll.pollingTimerSubscription = pollingTimerSubscription;
      poll.body = body;
    }
    return this.pollingHandlerMap[key];
  }

  /**
   * terminates the polling timer
   */
  protected terminatePolling(key, terminateHandler = true) {
    const poll = this.pollingHandlerMap[key];
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
