import { Component, OnDestroy, OnInit } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { environment } from '@env/environment';
import { Store } from '@ngrx/store';
import { takeUntil } from 'rxjs/operators/takeUntil';
import { Subject } from 'rxjs/Subject';

import { actionChangeTheme, selectorSettings } from '../settings.reducer';

@Component({
  selector: 'app-settings',
  templateUrl: './settings.component.html',
  styleUrls: ['./settings.component.scss']
})
export class SettingsComponent extends BaseComponent implements OnInit, OnDestroy {
  private unsubscribe$: Subject<void> = new Subject<void>();
  theme: string;
  envUrl: string;
  envPort: string;
  startState: boolean;
  startStateIdle: boolean;

  themes = [
    { value: 'DEFAULT-THEME', label: 'Default' },
    { value: 'LIGHT-THEME', label: 'Light' },
    { value: 'BLACK-THEME', label: 'Black' }
  ];

  constructor(private store: Store<any>, protected _controllerService: ControllerService) {
    super(_controllerService);
    store
      .select(selectorSettings)
      .pipe(takeUntil(this.unsubscribe$))
      .subscribe(({ theme }) => (this.theme = theme));
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
        'component': 'SettingsComponent', 'state':
          Eventtypes.COMPONENT_INIT
      });
      this.envUrl = environment.server_url;
      this.envPort = environment.server_port;
      this.startState = this._controllerService.useRealData;
      this.startStateIdle = this._controllerService.enableIdle;
    }
  }

  ngOnDestroy(): void {
    this.unsubscribe$.next();
    this.unsubscribe$.complete();
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'SettingsComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  onThemeSelect({ value }) {
    this.store.dispatch(actionChangeTheme(value));
  }

  setEnvironmentUrl(url) {
    url = url.trim();
    if (url === '') {
      return;
    }
    environment.server_url = url;
    this.envUrl = url;
  }

  setEnvironmentPort(port) {
    port = port.trim();
    if (port === '') {
      return;
    }
    environment.server_port = port;
    this.envPort = port;
  }

  toggleHandlerEndpoint($event) {
    if ($event.checked) {
      this._controllerService.useRealData = true;
    } else {
      this._controllerService.useRealData = false;
    }
  }

  toggleHandlerIdle($event) {
    if ($event.checked) {
      this._controllerService.enableIdle = true;
    } else {
      this._controllerService.enableIdle = false;
    }
  }
}
