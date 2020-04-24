import { HttpClientTestingModule } from '@angular/common/http/testing';
import { ChangeDetectorRef } from '@angular/core';
import { ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { LazyrenderComponent } from '../lazyrender/lazyrender.component';
import { SorticonComponent } from '../sorticon/sorticon.component';
import { RoleGuardDirective } from '../directives/roleGuard.directive';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService as AuthServiceGen } from '@app/services/generated/auth.service';
import { AuthService } from '@app/services/auth.service';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule } from '@angular/forms';
import { AdvancedSearchComponent } from '../advanced-search/advanced-search.component';
import { WidgetsModule } from 'web-app-framework';
import { SafeStylePipe} from '../Pipes/SafeStyle.pipe';
import { PentableComponent } from './pentable.component';


describe('PentableComponent', () => {
  let component: PentableComponent;
  let fixture: ComponentFixture<PentableComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [
        PentableComponent,
        LazyrenderComponent,
        SorticonComponent,
        RoleGuardDirective,
        AdvancedSearchComponent,
        SafeStylePipe
      ],
      imports: [
        NoopAnimationsModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        FlexLayoutModule,
        FormsModule,
        WidgetsModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        AuthService,
        AuthServiceGen,
        LogPublishersService,
        MatIconRegistry,
        MessageService,
        ChangeDetectorRef,
        UIConfigsService
      ]
    });
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(PentableComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should be created', () => {
    expect(component).toBeTruthy();
  });
});
