import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { WidgetsModule } from 'web-app-framework';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { SecurityService } from '@app/services/generated/security.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { MessageService } from '@app/services/message.service';
import { AuthService } from '@app/services/auth.service';
import { NewsecurityappComponent } from './newsecurityapp.component';
import { SearchService } from '@app/services/generated/search.service';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import { CommonModule } from '@angular/common';
import { MatIconRegistry } from '@angular/material';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import { EventsService } from '@app/services/events.service';



describe('NewsecurityappComponent', () => {
  let component: NewsecurityappComponent;
  let fixture: ComponentFixture<NewsecurityappComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [NewsecurityappComponent],
      imports: [
        CommonModule,
        RouterTestingModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        WidgetsModule,
        SharedModule,
        PrimengModule,
        ReactiveFormsModule,

        FormsModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        EventsService,
        MatIconRegistry,
        SecurityService,
        SearchService,
        UIConfigsService,
        LicenseService,
        AuthService,
        MessageService
      ],
    });
      });

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NewsecurityappComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NewsecurityappComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
