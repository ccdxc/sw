import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ModalcontentComponent } from './modalcontent.component';

describe('ModalcontentComponent', () => {
  let component: ModalcontentComponent;
  let fixture: ComponentFixture<ModalcontentComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ModalcontentComponent],
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalcontentComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
