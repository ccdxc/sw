import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { VeniceResponse } from '@app/models/frontend/shared/veniceresponse.interface';
import { ControllerService } from '@app/services/controller.service';
import { Workloadv1Service } from '@sdk/v1/services/generated/workloadv1.service';
import { Observable } from 'rxjs';
import { environment } from '../../../environments/environment';
import { Utility } from '../../common/Utility';
import { GenServiceUtility } from './GenUtility';
import { UIConfigsService } from '../uiconfigs.service';
import { NEVER } from 'rxjs';
import { MethodOpts } from '@sdk/v1/services/generated/abstract.service';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';

@Injectable()
export class WorkloadService extends Workloadv1Service {

  // Attributes used by generated services
  protected O_Tenant: string = this.getTenant();
  protected baseUrlAndPort = window.location.protocol + '//' + window.location.hostname + ':' + window.location.port;
  protected oboeServiceMap: { [method: string]: Observable<VeniceResponse> } = {};
  protected serviceUtility: GenServiceUtility;

  constructor(protected _http: HttpClient,
              protected _controllerService: ControllerService,
              protected uiconfigsService: UIConfigsService) {
      super(_http);
      this.serviceUtility = new GenServiceUtility(
        _http,
        (payload) => { this.publishAJAXStart(payload); },
        (payload) => { this.publishAJAXEnd(payload); }
      );
      this.serviceUtility.setId(this.getClassName());
      this.createListWorkloadCache();
  }

  /**
   * Get the service class-name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  protected publishAJAXStart(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_START, eventPayload);
  }

  protected publishAJAXEnd(eventPayload: any) {
    this._controllerService.publish(Eventtypes.AJAX_END, eventPayload);
  }

  protected createDataCache<T>(constructor: any, key: string, listFn: () => Observable<VeniceResponse>, watchFn: (query: any) => Observable<VeniceResponse>) {
    return this.serviceUtility.createDataCache(constructor, key, listFn, watchFn);
  }

  protected getFromDataCache(kind: string, createCacheFn: any) {
    return this.serviceUtility.handleListFromCache(kind, createCacheFn);
  }

  protected invokeAJAX(method: string, url: string, payload: any, opts: MethodOpts, forceReal: boolean = false): Observable<VeniceResponse> {

    const key = this.serviceUtility.convertEventID(opts);
    if (!this.uiconfigsService.isAuthorized(key)) {
      return NEVER;
    }
    const isOnline = !this.isToMockData() || forceReal;
    return this.serviceUtility.invokeAJAX(method, url, payload, opts.eventID, isOnline);
  }

  /**
   * Override-able api
   */
  public isToMockData(): boolean {
    const isUseRealData = this._controllerService.useRealData;
    return (!isUseRealData) ? isUseRealData : environment.isRESTAPIReady;
  }

  /**
   * Get login user tenant information
   */
  getTenant(): string {
    return Utility.getInstance().getTenant();
  }
}
