// SPDX-License-Identifier: GPL-2.0-only
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved.
// Published in Xo666/mainline-instantnoodle, commit 417fa473cdc7.

#include <linux/backlight.h>
#include <linux/bits.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/display/drm_dsc.h>
#include <drm/display/drm_dsc_helper.h>
#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

enum samsung_amb655uv01_supply {
	SAMSUNG_AMB655UV01_VCI,
	SAMSUNG_AMB655UV01_VDDR,
	SAMSUNG_AMB655UV01_VDDI,
	SAMSUNG_AMB655UV01_VDDD,
	SAMSUNG_AMB655UV01_NUM_SUPPLIES,
};

struct samsung_amb655uv01 {
	struct drm_panel panel;
	struct drm_connector *connector;
	struct mipi_dsi_device *dsi;
	struct drm_dsc_config dsc;
	/* Serialize panel commands with backlight updates. */
	struct mutex transfer_lock;
	struct gpio_desc *reset_gpio;
	struct regulator_bulk_data supplies[SAMSUNG_AMB655UV01_NUM_SUPPLIES];
	u8 enabled_supplies;
};

static inline
struct samsung_amb655uv01 *to_samsung_amb655uv01(struct drm_panel *panel)
{
	return container_of(panel, struct samsung_amb655uv01, panel);
}

static void samsung_amb655uv01_write(struct mipi_dsi_multi_context *ctx,
				     const void *data, size_t len)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	const struct mipi_dsi_host_ops *ops = dsi->host->ops;
	struct mipi_dsi_msg msg = {
		.channel = dsi->channel,
		.type = MIPI_DSI_DCS_LONG_WRITE,
		.tx_buf = data,
		.tx_len = len,
	};
	ssize_t ret;

	if (ctx->accum_err)
		return;
	if (!ops || !ops->transfer) {
		ctx->accum_err = -EOPNOTSUPP;
		return;
	}

	/* The factory sequence uses long packets even for two-byte writes. */
	if (dsi->mode_flags & MIPI_DSI_MODE_LPM)
		msg.flags |= MIPI_DSI_MSG_USE_LPM;
	ret = ops->transfer(dsi->host, &msg);
	if (ret < 0) {
		ctx->accum_err = ret;
		dev_err(&dsi->dev, "Failed to write panel command: %zd\n", ret);
	}
}

#define samsung_amb655uv01_write_seq(ctx, seq...) do { \
	static const u8 data[] = { seq }; \
	samsung_amb655uv01_write(ctx, data, ARRAY_SIZE(data)); \
} while (0)

static void samsung_amb655uv01_reset(struct samsung_amb655uv01 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int samsung_amb655uv01_on(struct samsung_amb655uv01 *ctx, unsigned int hz)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0x9d, 0x01);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0x9e,
				     0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x09,
				     0x60, 0x04, 0x38, 0x00, 0x30, 0x02, 0x1c,
				     0x02, 0x1c, 0x02, 0x00, 0x02, 0x52, 0x00,
				     0x20, 0x04, 0x23, 0x00, 0x07, 0x00, 0x0f,
				     0x02, 0x8e, 0x02, 0x1f, 0x18, 0x00, 0x10,
				     0xf0, 0x03, 0x0c, 0x20, 0x00, 0x06, 0x0b,
				     0x0b, 0x33, 0x0e, 0x1c, 0x2a, 0x38, 0x46,
				     0x54, 0x62, 0x69, 0x70, 0x77, 0x79, 0x7b,
				     0x7d, 0x7e, 0x01, 0x02, 0x01, 0x00, 0x09,
				     0x40, 0x09, 0xbe, 0x19, 0xfc, 0x19, 0xfa,
				     0x19, 0xf8, 0x1a, 0x38, 0x1a, 0x78, 0x1a,
				     0xb6, 0x2a, 0xb6, 0x2a, 0xf4, 0x2a, 0xf4,
				     0x4b, 0x34, 0x63, 0x74);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 20);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x85);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x01);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x77);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x03);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x10, 0x40, 0x10, 0x40);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x3b);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x25);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, MIPI_DCS_SET_TEAR_ON, 0x00);
	/* Disable the command page for the following address writes. */
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_set_column_address_multi(&dsi_ctx, 0x0000, 0x0437);
	mipi_dsi_dcs_set_page_address_multi(&dsi_ctx, 0x0000, 0x095f);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x05);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xd7, 0x07, 0x02, 0x40);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x85);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x02);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf8, 0xc4);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x04);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf8,
				     0x41, 0x47, 0x11, 0xb8, 0xb8);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x02);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf8, 0xcc);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x08);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf2, 0xa0);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	mipi_dsi_msleep(&dsi_ctx, 110);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf5, 0x87);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x06);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb7, 0x01);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x05);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb7, 0x13);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xb7, 0x01, 0x37);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x20);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	if (hz == 90)
		samsung_amb655uv01_write_seq(&dsi_ctx, 0x60, 0x10);
	else
		samsung_amb655uv01_write_seq(&dsi_ctx, 0x60, 0x00);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	samsung_amb655uv01_write_seq(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x00);
	samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	if (hz == 60) {
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0x5a, 0x5a);
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x69);
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xe6, 0x02);
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xb0, 0x77);
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xe6, 0xb3);
		samsung_amb655uv01_write_seq(&dsi_ctx, 0xf0, 0xa5, 0xa5);
	}
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);

	return dsi_ctx.accum_err;
}

static int samsung_amb655uv01_off(struct samsung_amb655uv01 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

/* Atomic enable runs after the connector/CRTC states have been swapped. */
static unsigned int samsung_amb655uv01_get_refresh(struct samsung_amb655uv01 *ctx)
{
	if (ctx->connector && ctx->connector->state &&
	    ctx->connector->state->crtc && ctx->connector->state->crtc->state) {
		const struct drm_display_mode *mode =
			&ctx->connector->state->crtc->state->adjusted_mode;

		if (mode->clock)
			return drm_mode_vrefresh(mode) < 75 ? 60 : 90;
	}

	return 90;
}

static void samsung_amb655uv01_power_off(struct samsung_amb655uv01 *ctx)
{
	int i, ret;

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 1500);
	for (i = ARRAY_SIZE(ctx->supplies) - 1; i >= 0; i--) {
		if (!(ctx->enabled_supplies & BIT(i)))
			continue;
		ret = regulator_disable(ctx->supplies[i].consumer);
		if (ret)
			dev_warn(&ctx->dsi->dev, "Failed to disable %s: %d\n",
				 ctx->supplies[i].supply, ret);
		else
			ctx->enabled_supplies &= ~BIT(i);
	}
}

static void samsung_amb655uv01_power_cleanup(void *data)
{
	struct samsung_amb655uv01 *ctx = data;

	guard(mutex)(&ctx->transfer_lock);
	samsung_amb655uv01_power_off(ctx);
}

static int samsung_amb655uv01_prepare(struct drm_panel *panel)
{
	struct samsung_amb655uv01 *ctx = to_samsung_amb655uv01(panel);
	struct device *dev = &ctx->dsi->dev;
	struct drm_dsc_picture_parameter_set pps;
	unsigned int hz;
	int i, ret;

	guard(mutex)(&ctx->transfer_lock);
	/* VCI, VDDR, VDDI, then the digital-power enable, as in the stock table. */
	for (i = 0; i < ARRAY_SIZE(ctx->supplies); i++) {
		if (ctx->enabled_supplies & BIT(i))
			continue;
		ret = regulator_enable(ctx->supplies[i].consumer);
		if (ret) {
			dev_err(dev, "Failed to enable %s: %d\n",
				ctx->supplies[i].supply, ret);
			goto power_off;
		}
		ctx->enabled_supplies |= BIT(i);
		if (i == SAMSUNG_AMB655UV01_VDDI)
			usleep_range(2000, 2500);
	}

	samsung_amb655uv01_reset(ctx);
	hz = samsung_amb655uv01_get_refresh(ctx);
	ret = samsung_amb655uv01_on(ctx, hz);
	if (ret) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		goto power_off;
	}

	drm_dsc_pps_payload_pack(&pps, &ctx->dsc);
	ret = mipi_dsi_picture_parameter_set(ctx->dsi, &pps);
	if (ret < 0) {
		dev_err(dev, "Failed to transmit PPS: %d\n", ret);
		goto power_off;
	}
	ret = mipi_dsi_compression_mode(ctx->dsi, true);
	if (ret < 0) {
		dev_err(dev, "Failed to enable compression mode: %d\n", ret);
		goto power_off;
	}

	/* Retain the reference panel's PPS settling delay. */
	msleep(28);

	return 0;

power_off:
	samsung_amb655uv01_power_off(ctx);
	return ret;
}

static int samsung_amb655uv01_unprepare(struct drm_panel *panel)
{
	struct samsung_amb655uv01 *ctx = to_samsung_amb655uv01(panel);
	int ret;

	guard(mutex)(&ctx->transfer_lock);
	ret = samsung_amb655uv01_off(ctx);
	if (ret)
		dev_err(panel->dev, "Failed to un-initialize panel: %d\n", ret);
	samsung_amb655uv01_power_off(ctx);

	return 0;
}

static const struct drm_display_mode samsung_amb655uv01_modes[] = {
	{ /* 90Hz Mode */
		/* In command mode, horizontal blanking expresses transfer
		 * overhead. With DSC, msm replaces 1080 active pixels by 360.
		 * 636 * 2416 * 90 * 24 / 4 = 829751040 bit/s per lane,
		 * within 0.03% of this handset's stock 830000000 bit/s.
		 */
		.clock = (1080 + 52 + 24 + 200) * (2400 + 4 + 4 + 8) * 90 / 1000,
		.hdisplay = 1080,
		.hsync_start = 1080 + 52,
		.hsync_end = 1080 + 52 + 24,
		.htotal = 1080 + 52 + 24 + 200,
		.vdisplay = 2400,
		.vsync_start = 2400 + 4,
		.vsync_end = 2400 + 4 + 4,
		.vtotal = 2400 + 4 + 4 + 8,
		.width_mm = 70,
		.height_mm = 151,
		.type = DRM_MODE_TYPE_DRIVER,
	},
	{
		/* 954 compressed horizontal periods keeps the same link at60Hz. */
		.clock = (1080 + 52 + 24 + 518) * (2400 + 4 + 4 + 8) * 60 / 1000,
		.hdisplay = 1080,
		.hsync_start = 1080 + 52,
		.hsync_end = 1080 + 52 + 24,
		.htotal = 1080 + 52 + 24 + 518,
		.vdisplay = 2400,
		.vsync_start = 2400 + 4,
		.vsync_end = 2400 + 4 + 4,
		.vtotal = 2400 + 4 + 4 + 8,
		.width_mm = 70,
		.height_mm = 151,
		.type = DRM_MODE_TYPE_DRIVER,
	},
};

static int samsung_amb655uv01_get_modes(struct drm_panel *panel,
					struct drm_connector *connector)
{
	struct samsung_amb655uv01 *ctx = to_samsung_amb655uv01(panel);
	int count = 0;

	ctx->connector = connector;
	for (int i = 0; i < ARRAY_SIZE(samsung_amb655uv01_modes); i++) {
		count += drm_connector_helper_get_modes_fixed(connector,
						    &samsung_amb655uv01_modes[i]);
	}

	return count;
}

static const struct drm_panel_funcs samsung_amb655uv01_panel_funcs = {
	.prepare = samsung_amb655uv01_prepare,
	.unprepare = samsung_amb655uv01_unprepare,
	.get_modes = samsung_amb655uv01_get_modes,
};

static int samsung_amb655uv01_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	struct samsung_amb655uv01 *ctx = mipi_dsi_get_drvdata(dsi);
	unsigned long mode_flags;
	u16 brightness;
	int ret;

	guard(mutex)(&ctx->transfer_lock);
	mode_flags = dsi->mode_flags;
	brightness = backlight_get_brightness(bl);
	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	/* A failed transfer must not change the next command's link mode. */
	dsi->mode_flags = mode_flags;

	return ret;
}

static const struct backlight_ops samsung_amb655uv01_bl_ops = {
	.update_status = samsung_amb655uv01_bl_update_status,
	/* Use the backlight core's cached brightness. DCS 0x52 readback is
	 * unreliable on this panel and can leave the DSI command path wedged.
	 */
};

static struct backlight_device *
samsung_amb655uv01_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 360,
		.max_brightness = 2047,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &samsung_amb655uv01_bl_ops, &props);
}

static int samsung_amb655uv01_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct samsung_amb655uv01 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct samsung_amb655uv01, panel,
				   &samsung_amb655uv01_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ctx->dsi = dsi;
	mutex_init(&ctx->transfer_lock);
	mipi_dsi_set_drvdata(dsi, ctx);

	ctx->supplies[SAMSUNG_AMB655UV01_VCI].supply = "vci";
	ctx->supplies[SAMSUNG_AMB655UV01_VDDR].supply = "vddr";
	ctx->supplies[SAMSUNG_AMB655UV01_VDDI].supply = "vddio";
	ctx->supplies[SAMSUNG_AMB655UV01_VDDD].supply = "vddd";
	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset GPIO\n");

	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get panel supplies\n");

	ret = regulator_set_voltage(ctx->supplies[SAMSUNG_AMB655UV01_VCI].consumer,
				    3000000, 3008000);
	if (ret)
		return dev_err_probe(dev, ret, "VCI voltage vote failed\n");
	ret = regulator_set_load(ctx->supplies[SAMSUNG_AMB655UV01_VCI].consumer, 857000);
	if (ret)
		return dev_err_probe(dev, ret, "VCI load vote failed\n");
	ret = regulator_set_voltage(ctx->supplies[SAMSUNG_AMB655UV01_VDDR].consumer,
				    1300000, 1304000);
	if (ret)
		return dev_err_probe(dev, ret, "VDDR voltage vote failed\n");
	ret = regulator_set_load(ctx->supplies[SAMSUNG_AMB655UV01_VDDR].consumer, 62000);
	if (ret)
		return dev_err_probe(dev, ret, "VDDR load vote failed\n");
	ret = regulator_set_voltage(ctx->supplies[SAMSUNG_AMB655UV01_VDDI].consumer,
				    1800000, 1808000);
	if (ret)
		return dev_err_probe(dev, ret, "VDDI voltage vote failed\n");
	ret = regulator_set_load(ctx->supplies[SAMSUNG_AMB655UV01_VDDI].consumer, 62000);
	if (ret)
		return dev_err_probe(dev, ret, "VDDI load vote failed\n");
	ret = devm_add_action_or_reset(dev, samsung_amb655uv01_power_cleanup, ctx);
	if (ret)
		return ret;

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_DSC_ALL_SLICES_IN_PKT;

	ctx->panel.backlight = samsung_amb655uv01_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");
	ctx->panel.prepare_prev_first = true;

	drm_panel_add(&ctx->panel);

	/* This panel only supports DSC; unconditionally enable it */
	dsi->dsc = &ctx->dsc;

	ctx->dsc.dsc_version_major = 1;
	ctx->dsc.dsc_version_minor = 1;

	/* Both 540-pixel slices of each line travel in one DSI packet. */
	ctx->dsc.slice_height = 48;
	ctx->dsc.slice_width = 540;
	ctx->dsc.slice_count = 1080 / ctx->dsc.slice_width;
	ctx->dsc.bits_per_component = 8;
	ctx->dsc.bits_per_pixel = 8 << 4; /* 4 fractional bits */
	ctx->dsc.block_pred_enable = true;

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void samsung_amb655uv01_remove(struct mipi_dsi_device *dsi)
{
	struct samsung_amb655uv01 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id samsung_amb655uv01_of_match[] = {
	{ .compatible = "samsung,amb655uv01" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, samsung_amb655uv01_of_match);

static struct mipi_dsi_driver samsung_amb655uv01_driver = {
	.probe = samsung_amb655uv01_probe,
	.remove = samsung_amb655uv01_remove,
	.driver = {
		.name = "panel-samsung-amb655uv01",
		.of_match_table = samsung_amb655uv01_of_match,
	},
};
module_mipi_dsi_driver(samsung_amb655uv01_driver);

MODULE_DESCRIPTION("DRM driver for Samsung amb655uv01 panel");
MODULE_LICENSE("GPL");
