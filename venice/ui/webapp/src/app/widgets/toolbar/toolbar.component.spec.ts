/* ---------------------------------------------------
    Angular JS libraries
----------------------------------------------------- */
import {  ComponentFixture, TestBed, inject } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { RouterTestingModule } from '@angular/router/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

/* ---------------------------------------------------
    Venice App libraries
----------------------------------------------------- */
import { ToolbarComponent } from './toolbar.component';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { MatIconRegistry } from '@angular/material';
/* ---------------------------------------------------
    Third-party libraries
----------------------------------------------------- */
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { ToolbarData, ToolbarButton } from '@app/models/frontend/shared/toolbar.interface';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { By } from '@angular/platform-browser';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { AuthService } from '@app/services/auth.service';
import { HttpClientTestingModule } from '@angular/common/http/testing';

describe('ToolbarComponent', () => {
  let component: ToolbarComponent;
  let fixture: ComponentFixture<ToolbarComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [ToolbarComponent],
      imports: [
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        FormsModule,
        ReactiveFormsModule,
        HttpClientTestingModule
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService,
        UIConfigsService,
        LicenseService,
        AuthService
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(ToolbarComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should render breadcrumb', inject([ControllerService], (controller: ControllerService) => {
    expect(component.breadcrumb.length).toBe(1);
    expect(component.breadcrumb[0].label).toBe('');
    const toolbar: ToolbarData = {
      breadcrumb: [{ label: 'testing' }, { label: 'breadcrumb' }, { label: 'component' }]
    };
    controller.publish(Eventtypes.TOOLBAR_DATA_CHANGE, toolbar);
    expect(component.breadcrumb.length).toBe(3);
    expect(component.breadcrumb[0].label).toBe('testing');
    expect(component.breadcrumb[1].label).toBe('breadcrumb');
    expect(component.breadcrumb[2].label).toBe('component');
  }));

  it('should render buttons', inject([ControllerService], (controller: ControllerService) => {
    expect(component.buttons.length).toBe(0);
    const button1: ToolbarButton = {
      text: 'test button 1',
      cssClass: 'testClass1',
      callback: () => { }
    };
    const button2: ToolbarButton = {
      text: 'test button 2',
      cssClass: 'testClass2',
      callback: () => { }
    };
    const spy1 = spyOn(button1, 'callback');
    const spy2 = spyOn(button2, 'callback');
    const toolbar: ToolbarData = {
      buttons: [button1, button2]
    };
    controller.publish(Eventtypes.TOOLBAR_DATA_CHANGE, toolbar);
    // Changing the toolbar object shouldn't affect the set toolbar
    // since we should be making copies
    toolbar.buttons = [button1];
    fixture.detectChanges();
    expect(component.buttons.length).toBe(2);

    let buttons = fixture.debugElement.queryAll(By.css('.toolbar-button'));
    expect(buttons.length).toBe(2);
    expect(buttons[0].nativeElement.textContent).toContain('test button 1');
    expect(buttons[0].nativeElement.className).toContain('toolbar-button');
    expect(buttons[0].nativeElement.className).toContain('testClass1');
    buttons[0].nativeElement.click();
    expect(spy1).toHaveBeenCalled();
    expect(spy2).toHaveBeenCalledTimes(0);

    expect(buttons[1].nativeElement.textContent).toContain('test button 2');
    expect(buttons[1].nativeElement.className).toContain('toolbar-button');
    expect(buttons[1].nativeElement.className).toContain('testClass2');

    buttons[1].nativeElement.click();
    expect(spy1).toHaveBeenCalledTimes(1);
    expect(spy2).toHaveBeenCalledTimes(1);


    // Checking computeClass attribute is called and applied
    let computedClass = 'computedDisabled';
    button2.computeClass = () => computedClass;
    toolbar.buttons = [button2];
    controller.publish(Eventtypes.TOOLBAR_DATA_CHANGE, toolbar);
    fixture.detectChanges();
    expect(component.buttons.length).toBe(1);
    buttons = fixture.debugElement.queryAll(By.css('.toolbar-button'));
    expect(buttons.length).toBe(1);
    expect(buttons[0].nativeElement.className).toContain('computedDisabled');

    computedClass = 'computedEnabled';
    fixture.detectChanges();
    expect(buttons[0].nativeElement.className).toContain('computedEnabled');

  }));
});
