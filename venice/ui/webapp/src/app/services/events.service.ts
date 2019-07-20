import { Injectable } from '@angular/core';
import { EventsService as EventGenService } from '@app/services/generated/events.service';
import { ControllerService } from '@app/services/controller.service';
import { HttpClient } from '@angular/common/http';
import { IEventsEvent, IEventsEventList, ApiListWatchOptions, IApiListWatchOptions, ApiListWatchOptions_sort_order } from '@sdk/v1/models/generated/events';
import { PollUtility, PollingInstance } from '@app/services/PollUtility';
import { Utility } from '@app/common/Utility';
import { UIConfigsService } from './uiconfigs.service';

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

@Injectable()
export class EventsService extends EventGenService {
  pollingUtility: PollUtility;
  POLLING_INTERVAL = 10000;
  events: Array<IEventsEvent> = [];
  constructor(protected _http: HttpClient,
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService) {
    super(_http, _controllerService, uiconfigsService);
    this.pollingUtility = new PollUtility(
      (key, body, useRealData) => {
        this.pollingFetchData(key, body, useRealData);
      },
      this.POLLING_INTERVAL);
  }

  pollEvents(key: string, body: IApiListWatchOptions = { 'sort-order': ApiListWatchOptions_sort_order.none }) {
    const poll: PollingInstance = this.pollingUtility.initiatePolling(key, body, true, this.POLLING_INTERVAL, 0);
    return poll.handler;
  }

  /**
   * Called when the timer ticks to fetch data
   * @param useRealData
   */
  protected pollingFetchData(key, body, useRealData): void {
    // remove previous poll if its there
    this.pollingUtility.pollingHandlerMap[key].pollSub.forEach( s => {
      s.unsubscribe();
    });
    this.pollingUtility.pollingHandlerMap[key].pollSub = [];

    const bodyObj = new ApiListWatchOptions(body);
    const sub = this.PostGetEvents(bodyObj).subscribe(
      (resp) => {
        const respBody = resp.body as IEventsEventList;
        let items = respBody.items;
        if (items == null) {
          items = [];
        }
        const poll = this.pollingUtility.pollingHandlerMap[key];
        if (poll == null || poll.handler == null) {
          // observable was terminated, we do nothing
          return;
        }
        if (poll.isFirstPoll) {
          // Don't append data since its the first poll
          poll.handler.next(respBody.items);
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
            let res = items.concat(currArray);
            res = res.filter( (e) => {
              if (e == null || e.meta == null) {
                return false;
              }
              return true;
            });
            res = Utility.getLodash().uniqBy(res, (e: IEventsEvent) => {
              return e.meta.uuid;
            });
            poll.handler.next(res);
            // Modify time selector to only get new data
            this.addModTimeSelector(items[0].meta['mod-time'], poll.body);
          }
        }
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster('Failed to get events', error);
        const poll = this.pollingUtility.pollingHandlerMap[key];
        if (poll == null || poll.handler == null) {
          return;
        }
        poll.handler.error(error);
      }
    );

    this.pollingUtility.pollingHandlerMap[key].pollSub.push(sub);
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

}
