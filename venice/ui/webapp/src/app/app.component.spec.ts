/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import { TestBed, async } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { OverlayContainer } from '@angular/cdk/overlay';

import { HttpClient } from '@angular/common/http';
import {HttpClientTestingModule} from '@angular/common/http/testing';
import {WidgetsModule} from 'web-app-framework';

import { Store } from '@ngrx/store';
import { Subject } from 'rxjs/Subject';
import { takeUntil } from 'rxjs/operators/takeUntil';
/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { AppComponent } from './app.component';
import { ControllerService } from './services/controller.service';
import { DatafetchService } from './services/datafetch.service';
import { LogService } from './services/logging/log.service';
import { LogPublishersService } from './services/logging/log-publishers.service';
import { AuthService } from './services/auth.service';
import { Eventtypes } from './enum/eventtypes.enum';
import { CoreModule } from '@app/core';

/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import {PrimengModule} from './lib/primeng.module';
import {MaterialdesignModule} from './lib/materialdesign.module';


describe('AppComponent', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [
        AppComponent
      ],
      imports: [
                // Other modules...
                HttpClientTestingModule,
                RouterTestingModule,
                FormsModule,
                PrimengModule,
                MaterialdesignModule,
                WidgetsModule,
                CoreModule

            ],
      providers: [
            ControllerService,
            DatafetchService,
            AuthService,
            LogService,
            LogPublishersService,
            Store,
            OverlayContainer
        ],
    });

    TestBed.compileComponents();
  });

  it('should create the app', async(() => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  }));


});
